#pragma once

#include <vector>
#include <cstddef>

namespace nbody {

class DomainDecomposition {
public:
    DomainDecomposition(int rank, int size);

    // Returns the start index (inclusive) for this rank
    std::size_t getLocalStart(std::size_t total_bodies) const;

    // Returns the number of bodies owned by this rank
    std::size_t getLocalCount(std::size_t total_bodies) const;

    // Helper to generate the arrays needed for MPI_Allgatherv / MPI_Scatterv
    // Returns {counts, displacements}
    std::pair<std::vector<int>, std::vector<int>> getcv(int total_bodies) const;

    int getRank() const { return rank_; }
    int getSize() const { return size_; }

private:
    int rank_;
    int size_;
};

} // namespace nbody