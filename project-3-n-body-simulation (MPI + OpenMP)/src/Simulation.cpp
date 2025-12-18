#include "nbody/Simulation.hpp"
#include "nbody/Constants.hpp"
#include <iostream>
#include <cmath>
#include <mpi.h>
#include <omp.h>
#include <iomanip>

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
    
    // Scatter data (Root -> Leaves)
    MPI_Scatterv(
        global_initial_bodies.data(), counts.data(), displs.data(), body_type,
        local_bodies_.data(), my_count, body_type,
        0, MPI_COMM_WORLD
    );

    // Prepare global buffer
    global_bodies_snapshot_.resize(total_bodies);
}

void Simulation::run(int steps, double dt) {
    int rank = domain_.getRank();
    int size = domain_.getSize();
    int total_bodies = static_cast<int>(global_bodies_snapshot_.size());
    
    auto [counts, displs] = domain_.getcv(total_bodies);
    MPI_Datatype body_type = Body::mpiType();

    // ---------------------------------------------------------
    // Performance Header
    // ---------------------------------------------------------
    if (rank == 0) {
        std::cout << "========================================\n";
        std::cout << " Hybrid N-Body Simulation \n";
        std::cout << "========================================\n";
        std::cout << " Bodies     : " << total_bodies << "\n";
        std::cout << " Steps      : " << steps << "\n";
        std::cout << " dt         : " << dt << "\n";
        std::cout << " MPI Ranks  : " << size << "\n";
        #pragma omp parallel
        {
            #pragma omp single
            std::cout << " OMP Threads: " << omp_get_num_threads() << " per rank\n";
        }
        std::cout << "----------------------------------------\n";
    }

    // Barrier to ensure all ranks start timing together
    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    // ---------------------------------------------------------
    // Main Loop
    // ---------------------------------------------------------
    for (int step = 0; step < steps; ++step) {
        
        // 1. Communication (All-to-All)
        MPI_Allgatherv(
            local_bodies_.data(), static_cast<int>(local_bodies_.size()), body_type,
            global_bodies_snapshot_.data(), counts.data(), displs.data(), body_type,
            MPI_COMM_WORLD
        );

        // 2. Compute Forces (Compute Bound)
        computeForces();

        // 3. Integrate (Memory Bound)
        updatePositions(dt);
        
        // Progress bar (only root)
        if (rank == 0 && (step % 10 == 0 || step == steps - 1)) {
            // Calculate progress percentage
            float progress = (float)(step + 1) / steps * 100.0f;
            std::cout << "\r Progress: " << std::fixed << std::setprecision(1) 
                      << progress << "%" << std::flush;
        }
    }

    // Barrier to ensure timing correctness
    MPI_Barrier(MPI_COMM_WORLD);
    double end_time = MPI_Wtime();
    double elapsed = end_time - start_time;

    // ---------------------------------------------------------
    // Performance Reporting
    // ---------------------------------------------------------
    if (rank == 0) {
        std::cout << "\n----------------------------------------\n";
        std::cout << " Simulation Complete.\n";
        std::cout << " Wall Time  : " << std::fixed << std::setprecision(4) << elapsed << " s\n";
        
        // Compute stats
        double interactions_per_step = (double)total_bodies * total_bodies;
        double total_interactions = interactions_per_step * steps;
        double interactions_per_sec = total_interactions / elapsed;
        
        std::cout << " Performance: " << std::scientific << std::setprecision(2) 
                  << interactions_per_sec << " interactions/s\n";
        std::cout << "========================================\n";
    }
}

void Simulation::computeForces() {
    int n_local = static_cast<int>(local_bodies_.size());
    int n_global = static_cast<int>(global_bodies_snapshot_.size());
    
    // Resize forces vectors if needed (safety check)
    if (forces_x_.size() != local_bodies_.size()) {
        forces_x_.resize(n_local);
        forces_y_.resize(n_local);
    }
    
    // Pre-calculate global offset for this rank
    size_t my_offset = domain_.getLocalStart(n_global);

    // OpenMP Region
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n_local; ++i) {
        double fx = 0.0;
        double fy = 0.0;
        
        // Load local body data into registers
        double xi = local_bodies_[i].x;
        double yi = local_bodies_[i].y;
        double mi = local_bodies_[i].mass;
        
        // Global index of current body
        size_t global_i = my_offset + i;

        // Inner Loop: Interact with ALL global bodies
        for (int j = 0; j < n_global; ++j) {
            // Avoid self-interaction
            if (static_cast<int>(global_i) == j) continue;

            double dx = global_bodies_snapshot_[j].x - xi;
            double dy = global_bodies_snapshot_[j].y - yi;
            
            // Softened distance
            double dist_sq = dx*dx + dy*dy + (SOFTENING * SOFTENING);
            
            // 1/r^3
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

    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n_local; ++i) {
        double inv_mass = 1.0 / local_bodies_[i].mass;
        double ax = forces_x_[i] * inv_mass;
        double ay = forces_y_[i] * inv_mass;

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

    // Note: data() returns pointer to non-const, safe for const_cast if needed by old MPI
    MPI_Gatherv(
        const_cast<Body*>(local_bodies_.data()), static_cast<int>(local_bodies_.size()), body_type,
        final_state.data(), counts.data(), displs.data(), body_type,
        0, MPI_COMM_WORLD
    );

    return final_state;
}

} // namespace nbody