cmake_minimum_required(VERSION 3.20)

project(HybridNBody 
    VERSION 0.1.0 
    LANGUAGES CXX
)

# -----------------------------------------------------------------------------
# 1. Configuration & Dependencies
# -----------------------------------------------------------------------------

# Use C++17 Standard (Required for structure bindings, etc.)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable "Safe" Warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find MPI (Required for distributed memory parallelism)
find_package(MPI REQUIRED)

# Find OpenMP (Required for shared memory parallelism)
find_package(OpenMP REQUIRED)

# -----------------------------------------------------------------------------
# 2. Library Definition (The Core Logic)
# -----------------------------------------------------------------------------

# We create an object library or static library for the core logic.
# This makes it easier to link unit tests later.
add_library(nbody_core
    src/Body.cpp
    src/DomainDecomposition.cpp
    src/IO.cpp
    src/Simulation.cpp
)

# Include directories
target_include_directories(nbody_core PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
)

# Link MPI and OpenMP to the core library
target_link_libraries(nbody_core PUBLIC 
    MPI::MPI_CXX
    OpenMP::OpenMP_CXX
)

# -----------------------------------------------------------------------------
# 3. Executable Definition (The Application)
# -----------------------------------------------------------------------------

add_executable(nbody_sim src/main.cpp)

# Link against our core library
target_link_libraries(nbody_sim PRIVATE nbody_core)

# -----------------------------------------------------------------------------
# 4. Status Output
# -----------------------------------------------------------------------------
message(STATUS "Build configured for Hybrid N-Body Simulation")
message(STATUS "MPI Found: ${MPI_CXX_FOUND}")
message(STATUS "OpenMP Found: ${OpenMP_CXX_FOUND}")