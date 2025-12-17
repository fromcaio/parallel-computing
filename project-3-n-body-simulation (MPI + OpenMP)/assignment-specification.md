# Parallel N-Body Simulation (Hybrid MPI + OpenMP)

---

## Overview

In this problem set, you will design and implement a **parallel simulation of the N-body problem**, a classic application in scientific computing. The simulation models the evolution of a system of bodies interacting through gravitational forces.

Your implementation must use a **hybrid parallel programming model**, combining **MPI** for distributed-memory parallelism and **OpenMP** for shared-memory parallelism.

This assignment emphasizes:
- Performance analysis and profiling
- Data decomposition and communication patterns
- Hybrid parallel design
- Scalability and speedup evaluation

---

## Learning Objectives

By completing this assignment, you will be able to:

- Identify computational bottlenecks in scientific simulations
- Design efficient data decompositions for O(N²) problems
- Implement collective communication patterns using MPI
- Combine MPI and OpenMP in a single hybrid application
- Analyze parallel performance, scalability, and communication overheads

---

## Problem Description

You will simulate the motion of **N bodies in two-dimensional space** over a fixed number of time steps. Each body is characterized by its mass, position, and velocity.

At each time step `dt`, the simulation must perform the following operations:

1. **Force Computation**  
   For each body *i*, compute the total gravitational force exerted on it by all other bodies *j ≠ i*, according to **Newton’s Law of Universal Gravitation**.

2. **Velocity Update**  
   Use the computed force and **Newton’s Second Law** (*F = ma*) to update the velocity of each body.

3. **Position Update**  
   Update the position of each body using the newly computed velocity.

This sequence is repeated for `N_STEPS` iterations.

---

## Parallelization Strategy

### Computational Complexity

The force computation dominates the runtime and has **O(N²)** complexity. Efficient parallelization of this phase is critical for achieving good performance.

### Data Decomposition

A recommended approach is to **partition the set of bodies across MPI processes**. Each process is responsible for computing forces and updating positions for its local subset of bodies.

### Communication Pattern

To compute forces, each process requires the **position and mass of every body** in the system. Therefore, at each time step, processes must exchange body data using an **all-to-all communication pattern**.

You are encouraged to use collective MPI operations such as:

- `MPI_Allgather`
- `MPI_Allgatherv` (if load imbalance exists)

Within each MPI process, OpenMP should be used to parallelize the force computation over threads.

---

## Input and Output Specification

### Input Format

The input file contains:

1. An integer **N** — number of bodies  
2. An integer **N_STEPS** — number of simulation steps  
3. A floating-point value **dt** — time step size  
4. **N lines**, each describing one body:

```

mass  pos_x  pos_y  vel_x  vel_y

```

### Output Format

At the end of the simulation, your program must write an output file containing the **final state of all N bodies**, using the same format as the input body descriptions.

---

## Implementation Requirements

- The code must compile and run using standard MPI and OpenMP toolchains.
- The hybrid MPI + OpenMP model must be clearly evident in the implementation.
- The program must work correctly for varying values of **N**, **N_STEPS**, and number of MPI processes.
- Correctness is required before performance will be evaluated.

---

## Performance Analysis and Report

You must submit a **technical report** addressing the following components:

### 1. Sequential Baseline

- Implement and profile a sequential version.
- Identify and quantify the main performance bottlenecks.

### 2. Parallel Design

- Describe your data decomposition strategy.
- Explain the all-to-all communication pattern and its necessity.
- Justify your choice of MPI collectives and OpenMP pragmas.

### 3. Parallel Implementation

- Discuss how work is divided among MPI processes and OpenMP threads.
- Explain how communication and computation are interleaved at each time step.

### 4. Evaluation and Scalability

- Measure speedup and efficiency for increasing numbers of MPI processes.
- Analyze how communication overhead impacts scalability.
- Discuss strong and/or weak scaling behavior.

Include plots and tables where appropriate.

---

## Submission Instructions

Submit the following files via the course portal:

1. **Report (PDF)**  
   - Clearly structured
   - Includes figures, performance plots, and discussion

2. **Source Code (compressed archive)**  
   - C or C++ source files
   - Build instructions (e.g., Makefile or CMake)

---

## Notes

- Numerical stability techniques (e.g., softening parameters) may be used but must be documented.
- You are encouraged to test your code on multiple hardware configurations if available.
- Clarity, correctness, and performance all matter.

---

**Good luck, and profile early.**
