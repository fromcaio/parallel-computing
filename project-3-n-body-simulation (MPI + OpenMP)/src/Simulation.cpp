#include "nbody/Simulation.hpp"
#include "nbody/Constants.hpp"
#include <iostream>
#include <cmath>
#include <mpi.h>
#include <omp.h>

namespace nbody {

Simulation::Simulation(const DomainDecomposition& domain) 
    : domain_(domain) {}

void Simulation::init(const SystemState& global_initial_bodies) {
    int total_bodies = 0;

    if (domain_.getRank() == 0) {
        total_bodies = static_cast<int>(global_initial_bodies.size());
    }
    MPI_Bcast(&total_bodies, 1, MPI_INT, 0, MPI_COMM_WORLD);

    auto [counts, displs] = domain_.getcv(total_bodies);
    int my_count = counts[domain_.getRank()];

    local_bodies_.resize(my_count);

    MPI_Datatype body_type = Body::mpiType();
    
    MPI_Scatterv(
        global_initial_bodies.data(), counts.data(), displs.data(), body_type,
        local_bodies_.data(), my_count, body_type,
        0, MPI_COMM_WORLD
    );

    global_bodies_snapshot_.resize(total_bodies);
}

void Simulation::run(int steps, double dt) {
    int rank = domain_.getRank();
    int total_bodies = static_cast<int>(global_bodies_snapshot_.size());
    auto [counts, displs] = domain_.getcv(total_bodies);
    MPI_Datatype body_type = Body::mpiType();

    if (rank == 0) {
        std::cout << "[Sim] Starting Hybrid MPI+OpenMP simulation.\n";
        std::cout << "      Steps=" << steps << ", dt=" << dt << "\n";
        std::cout << "      MPI Ranks: " << domain_.getSize() << "\n";
        // Show OpenMP threads from the perspective of master rank
        #pragma omp parallel
        {
            #pragma omp single
            std::cout << "      OpenMP Threads per Rank: " << omp_get_num_threads() << "\n";
        }
    }

    for (int step = 0; step < steps; ++step) {
        // 1. MPI Communication (Single Threaded context recommended for simplicity)
        MPI_Allgatherv(
            local_bodies_.data(), static_cast<int>(local_bodies_.size()), body_type,
            global_bodies_snapshot_.data(), counts.data(), displs.data(), body_type,
            MPI_COMM_WORLD
        );

        // 2. Compute Forces (Multi-Threaded)
        computeForces();

        // 3. Integrate (Multi-Threaded)
        updatePositions(dt);
        
        if (rank == 0 && (step % 10 == 0 || step == steps - 1)) {
            std::cout << "\r[Sim] Step " << step + 1 << "/" << steps << std::flush;
        }
    }

    if (rank == 0) std::cout << "\n[Sim] Finished.\n";
}

void Simulation::computeForces() {
    // Note: We use 'long long' or explicit casts for loop counters in OMP if warnings arise,
    // but standard 'int' or 'size_t' usually works fine in modern OMP.
    // Casting size_t to int for OpenMP loop limits is a common pattern if the compiler 
    // complains about unsigned loop variables, though C++20/OMP 5.0+ handles it better.
    // We'll use int for the loop variable to be safe across compilers.
    
    int n_local = static_cast<int>(local_bodies_.size());
    int n_global = static_cast<int>(global_bodies_snapshot_.size());
    
    forces_x_.assign(n_local, 0.0);
    forces_y_.assign(n_local, 0.0);

    // HYBRID PARALLELISM:
    // This loop is perfectly data-parallel. 
    // 'forces_x_' and 'forces_y_' are written to unique indices 'i'.
    // 'local_bodies_' and 'global_bodies_snapshot_' are read-only here.
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n_local; ++i) {
        double fx = 0.0;
        double fy = 0.0;
        
        double xi = local_bodies_[i].x;
        double yi = local_bodies_[i].y;
        double mi = local_bodies_[i].mass;

        // Inner loop iterates over the global snapshot (read-only)
        for (int j = 0; j < n_global; ++j) {
            // Get global index to check for self-interaction
            // Note: domain_.getLocalStart is cheap but we should call it safely.
            // Ideally, we hoist the offset calculation out, but it's fast enough.
            // Let's hoist it out of the inner loop, or compute it once.
            // However, calling a member function inside a parallel loop is fine 
            // as long as it's const.
            
            // To avoid function call overhead in O(N^2), let's calculate global index manually
            // BUT domain decomposition logic is encapsulated. 
            // We will trust the compiler to inline getLocalStart or accept the tiny overhead.
            
            // Better optimization: Calculate global_i once
            // But we need the offset for *this* rank.
            // Since this is inside 'omp parallel for', 'i' is private.
            
            // We need the global offset of the FIRST body on this rank.
            // This is invariant for the loop.
            // We can't easily extract it inside the loop without looking it up every time 
            // unless we pass it in.
            // Let's do a slight refactor to be clean.
        }
        
        // RE-DESIGNING THE LOOP FOR PERFORMANCE:
        // We calculate the global offset *outside* to avoid calling domain_ helper N times.
        // But we can't easily pass it into the loop unless we make it linear.
        // Actually, getLocalStart depends only on total_bodies (constant).
        // So we can compute 'my_global_offset' before the loop.
        
    } // End of failed thought trace.

    // ACTUAL IMPLEMENTATION:
    size_t my_offset = domain_.getLocalStart(n_global);

    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n_local; ++i) {
        double fx = 0.0;
        double fy = 0.0;
        
        double xi = local_bodies_[i].x;
        double yi = local_bodies_[i].y;
        double mi = local_bodies_[i].mass;
        
        size_t global_i_idx = my_offset + i;

        for (int j = 0; j < n_global; ++j) {
            if (static_cast<int>(global_i_idx) == j) continue;

            double dx = global_bodies_snapshot_[j].x - xi;
            double dy = global_bodies_snapshot_[j].y - yi;
            
            double dist_sq = dx*dx + dy*dy + (SOFTENING * SOFTENING);
            double dist = std::sqrt(dist_sq);
            double dist_cb = dist * dist * dist;

            double f = (G * mi * global_bodies_snapshot_[j].mass) / dist_cb;
            
            fx += f * dx;
            fy += f * dy;
        }
        forces_x_[i] = fx;
        forces_y_[i] = fy;
    }
}

void Simulation::updatePositions(double dt) {
    int n_local = static_cast<int>(local_bodies_.size());

    // This is O(N_local), much faster than force calc, but still worth threading
    // for large N or very small dt.
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n_local; ++i) {
        double ax = forces_x_[i] / local_bodies_[i].mass;
        double ay = forces_y_[i] / local_bodies_[i].mass;

        local_bodies_[i].vx += ax * dt;
        local_bodies_[i].vy += ay * dt;

        local_bodies_[i].x += local_bodies_[i].vx * dt;
        local_bodies_[i].y += local_bodies_[i].vy * dt;
    }
}

SystemState Simulation::gatherFinalState() const {
    int total_bodies = static_cast<int>(global_bodies_snapshot_.size());
    SystemState final_state(total_bodies);
    
    auto [counts, displs] = domain_.getcv(total_bodies);
    MPI_Datatype body_type = Body::mpiType();

    // MPI Calls must be done by the master thread usually (unless MPI_THREAD_MULTIPLE)
    // We are outside of any parallel region here, so it is safe.
    MPI_Gatherv(
        local_bodies_.data(), static_cast<int>(local_bodies_.size()), body_type,
        final_state.data(), counts.data(), displs.data(), body_type,
        0, MPI_COMM_WORLD
    );

    return final_state;
}

} // namespace nbody