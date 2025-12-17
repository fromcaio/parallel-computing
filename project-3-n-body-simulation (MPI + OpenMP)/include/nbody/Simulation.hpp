#pragma once

#include "nbody/Body.hpp"
#include "nbody/DomainDecomposition.hpp"
#include <vector>

namespace nbody {

class Simulation {
public:
    Simulation(const DomainDecomposition& domain);

    // Distributed Initialization:
    // Takes the full set of bodies (only valid on Rank 0) 
    // and scatters them to local storage.
    void init(const SystemState& global_initial_bodies);

    void run(int steps, double dt);
    
    // Gathers all data to Rank 0 for output
    SystemState gatherFinalState() const;

private:
    DomainDecomposition domain_;
    
    // The slice of bodies this rank owns and updates
    SystemState local_bodies_;
    
    // A buffer to hold the positions of ALL bodies (refreshed every step)
    SystemState global_bodies_snapshot_;
    
    // Temporary forces
    std::vector<double> forces_x_;
    std::vector<double> forces_y_;
    
    void computeForces();
    void updatePositions(double dt);
};

} // namespace nbody