#include "nbody/Body.hpp"
#include <array>
#include <cstddef>

namespace nbody {

MPI_Datatype Body::mpiType() {
    static MPI_Datatype type = MPI_DATATYPE_NULL;
    
    if (type == MPI_DATATYPE_NULL) {
        // 1. Describe the internal structure
        int block_lengths[5] = {1, 1, 1, 1, 1};
        MPI_Aint displacements[5];
        MPI_Datatype types[5] = {MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE};

        // FIX: Zero-initialize to silence warnings about uninitialized usage in Get_address
        Body dummy{}; 
        
        MPI_Get_address(&dummy.x, &displacements[0]);
        MPI_Get_address(&dummy.y, &displacements[1]);
        MPI_Get_address(&dummy.vx, &displacements[2]);
        MPI_Get_address(&dummy.vy, &displacements[3]);
        MPI_Get_address(&dummy.mass, &displacements[4]);

        // Normalize relative to base address
        MPI_Aint base;
        MPI_Get_address(&dummy, &base);
        for(int i=0; i<5; i++) displacements[i] -= base;

        MPI_Datatype temp_type;
        MPI_Type_create_struct(5, block_lengths, displacements, types, &temp_type);

        // 2. Resize to handle alignment/padding
        MPI_Type_create_resized(temp_type, 0, sizeof(Body), &type);
        MPI_Type_commit(&type);
        
        MPI_Type_free(&temp_type);
    }
    return type;
}

} // namespace nbody