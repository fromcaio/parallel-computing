#include <iostream>
#include <mpi.h>
#include <string>
#include <algorithm> // For std::find

#include "nbody/Simulation.hpp"
#include "nbody/IO.hpp"
#include "nbody/DomainDecomposition.hpp"

std::string getCmdOption(char ** begin, char ** end, const std::string & option) {
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end) {
        return std::string(*itr);
    }
    return "";
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::string input_file = "input.txt";
    std::string output_file = "output.txt";

    std::string i_arg = getCmdOption(argv, argv + argc, "-i");
    if (!i_arg.empty()) input_file = i_arg;
    std::string o_arg = getCmdOption(argv, argv + argc, "-o");
    if (!o_arg.empty()) output_file = o_arg;

    nbody::SystemState initial_bodies;
    double dt = 0.1;
    int steps = 10;

    // 1. Rank 0 Reads Data
    if (rank == 0) {
        try {
            auto params = nbody::IO::readInput(input_file, initial_bodies);
            dt = params.first;
            steps = params.second;
        } catch (const std::exception& e) {
            std::cerr << "[Error] Rank 0 failed to read input: " << e.what() << "\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    // 2. Broadcast Config
    MPI_Bcast(&steps, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&dt, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // 3. Setup Simulation
    nbody::DomainDecomposition dom(rank, size);
    nbody::Simulation sim(dom);

    // 4. Distribute Data (Scatter)
    // Rank 0 passes the filled vector, others pass empty/ignored vector
    sim.init(initial_bodies);

    // 5. Run Parallel Simulation
    sim.run(steps, dt);

    // 6. Gather & Output
    if (rank == 0) {
        nbody::SystemState final_bodies = sim.gatherFinalState();
        nbody::IO::writeOutput(output_file, final_bodies, dt, steps);
    } else {
        // Non-root ranks still participate in the Gatherv inside this function
        sim.gatherFinalState();
    }

    MPI_Finalize();
    return 0;
}