#pragma once

#include "nbody/Body.hpp"
#include <string>

namespace nbody {

class IO {
public:
    // Reads input file: N, N_STEPS, dt, followed by bodies
    // Returns a pair: {dt, steps}
    // The bodies vector is populated by reference
    static std::pair<double, int> readInput(const std::string& filename, SystemState& bodies);

    // Writes the final state to a file
    static void writeOutput(const std::string& filename, const SystemState& bodies, double dt, int steps);
};

} // namespace nbody