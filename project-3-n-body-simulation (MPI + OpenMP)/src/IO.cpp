#include "nbody/IO.hpp"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <iomanip>

namespace nbody {

std::pair<double, int> IO::readInput(const std::string& filename, SystemState& bodies) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open input file: " + filename);
    }

    int n_bodies;
    int n_steps;
    double dt;

    // Read header: N, N_STEPS, dt
    if (!(file >> n_bodies >> n_steps >> dt)) {
        throw std::runtime_error("Error reading header from " + filename);
    }

    bodies.clear();
    bodies.reserve(n_bodies);

    // Read bodies: mass, pos_x, pos_y, vel_x, vel_y
    for (int i = 0; i < n_bodies; ++i) {
        Body b;
        if (!(file >> b.mass >> b.x >> b.y >> b.vx >> b.vy)) {
            throw std::runtime_error("Error reading body " + std::to_string(i));
        }
        bodies.push_back(b);
    }

    file.close();

    std::cout << "[IO] Loaded " << bodies.size() << " bodies. Steps=" << n_steps << ", dt=" << dt << "\n";
    return {dt, n_steps};
}

void IO::writeOutput(const std::string& filename, const SystemState& bodies, double dt, int steps) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open output file: " + filename);
    }

    // Write header matching input format so output can be reused
    file << bodies.size() << "\n";
    file << steps << "\n";
    file << std::scientific << std::setprecision(6) << dt << "\n";

    for (const auto& b : bodies) {
        file << b.mass << "\t" 
             << b.x << "\t" << b.y << "\t" 
             << b.vx << "\t" << b.vy << "\n";
    }

    file.close();
    std::cout << "[IO] Wrote " << bodies.size() << " bodies to " << filename << "\n";
}

} // namespace nbody