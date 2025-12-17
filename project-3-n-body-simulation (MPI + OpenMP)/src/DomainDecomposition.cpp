#include "nbody/DomainDecomposition.hpp"
#include <numeric>

namespace nbody {

DomainDecomposition::DomainDecomposition(int rank, int size)
    : rank_(rank), size_(size) {}

std::size_t DomainDecomposition::getLocalStart(std::size_t total_bodies) const {
    std::size_t remainder = total_bodies % size_;
    std::size_t count = total_bodies / size_;
    
    if (static_cast<std::size_t>(rank_) < remainder) {
        return rank_ * (count + 1);
    } else {
        return remainder * (count + 1) + (rank_ - remainder) * count;
    }
}

std::size_t DomainDecomposition::getLocalCount(std::size_t total_bodies) const {
    std::size_t remainder = total_bodies % size_;
    std::size_t count = total_bodies / size_;
    
    if (static_cast<std::size_t>(rank_) < remainder) {
        return count + 1;
    }
    return count;
}

std::pair<std::vector<int>, std::vector<int>> DomainDecomposition::getcv(int total_bodies) const {
    std::vector<int> counts(size_);
    std::vector<int> displs(size_);

    int current_disp = 0;
    int remainder = total_bodies % size_;
    int count = total_bodies / size_;

    for (int r = 0; r < size_; ++r) {
        displs[r] = current_disp;
        if (r < remainder) {
            counts[r] = count + 1;
        } else {
            counts[r] = count;
        }
        current_disp += counts[r];
    }
    return {counts, displs};
}

} // namespace nbody