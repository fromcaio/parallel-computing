#pragma once

namespace nbody {

// Gravitational Constant
constexpr double G = 1.0; 

// Default Softening parameter (to prevent division by zero)
constexpr double SOFTENING = 1e-9;

} // namespace nbody