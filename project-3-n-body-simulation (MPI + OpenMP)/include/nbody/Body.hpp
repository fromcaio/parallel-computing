#pragma once

#include <vector>
#include <mpi.h>

namespace nbody {

// POD structure aligned to 32 bytes
struct alignas(32) Body {
    double x, y;
    double vx, vy;
    double mass;
    
    // Static helper to create the MPI custom type
    static MPI_Datatype mpiType();
};

using SystemState = std::vector<Body>;

} // namespace nbody