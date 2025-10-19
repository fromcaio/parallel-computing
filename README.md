# 🧩 Parallel Computing Projects

A collection of **academic projects** developed for a *Parallel Computing* course.  
Each implementation explores a different model of parallel programming:

- 🚀 **MPI** (Message Passing Interface)  
- 🧵 **Pthreads** (POSIX Threads)  
- ⚡ **Hybrid** approach using **MPI + OpenMP**

---

## 🗂️ Repository Structure

Each project is contained in its own directory:

| Directory | Description |
|------------|-------------|
| `project-1-sieve-mpi/` | Sieve of Eratosthenes using MPI |
| `project-2-game-of-life-pthreads/` | Conway's Game of Life using Pthreads |
| `project-3-n-body-hybrid/` | N-Body Simulation using hybrid MPI + OpenMP |

---

## 🧱 Project Folder Layout

Each project follows the same structure for clarity and consistency:

```

/bin/           # Compiled executables
/obj/           # Object and dependency files (.o, .d)
/src/           # Source code (.c)
/src/include/   # Header files (.h)
/doc/           # Documentation (PDF specs, reports)
Makefile        # Build configuration

````

---

## 💡 Project Summaries

<details>
<summary><b>🧮 Project 1 — Sieve of Eratosthenes (MPI)</b></summary>

**Objective:** Efficiently find all prime numbers up to a given integer `N` using the Sieve of Eratosthenes algorithm.  
**Technology:** MPI  
**Parallel Model:** **Master–Slave**

#### Overview
1. The master process (rank 0) reads `N` and computes all primes up to `√N`.  
2. The master **broadcasts** this list of small primes to all slave processes.  
3. The interval `[2, N]` is divided into blocks, each assigned to a slave.  
4. Each slave sieves its block and identifies local primes.  
5. Slaves return their results to the master, which merges them into `primos.txt`.

</details>

---

<details>
<summary><b>🧬 Project 2 — Conway’s Game of Life (Pthreads)</b></summary>

**Objective:** Simulate Conway’s Game of Life on a 2D grid for a given number of generations.  
**Technology:** Pthreads  
**Parallel Model:** **Domain Decomposition**

#### Overview
1. The 2D grid is split into horizontal sections (row strips).  
2. Each thread updates one section of cells.  
3. Boundary cells require coordination between adjacent threads.  
4. **Barrier synchronization** ensures all threads finish reading one generation before writing the next, preventing race conditions.

</details>

---

<details>
<summary><b>🌌 Project 3 — N-Body Simulation (Hybrid MPI + OpenMP)</b></summary>

**Objective:** Simulate the N-body problem — the evolution of a system of bodies in 2D space under mutual gravitational forces.  
**Technology:** Hybrid **MPI + OpenMP**  
**Parallel Model:** **Data Decomposition**

#### Overview
1. **MPI (Distributed Memory):** Bodies are distributed among MPI processes.  
   Each process updates positions and velocities for its subset.  
2. The $O(N^2)$ force computation is parallelized using **all-to-all communication** (`MPI_Allgather`).  
3. **OpenMP (Shared Memory):** Within each MPI process, OpenMP parallelizes local force calculations across all cores.

</details>

---

## ⚙️ Build & Run Instructions

1. **Enter the project folder:**

   ```bash
   cd project-1-sieve-mpi/
    ```

2. **Compile the project:**

   ```bash
   make
   ```

3. **Run the executable:**

   ```bash
   ./bin/<executable_name>
   ```

4. **Clean up:**

   ```bash
   make clean
   ```

---

## 🧰 Compiler Requirements

| Project                            | Compiler            | Notes                                                            |
| ---------------------------------- | ------------------- | ---------------------------------------------------------------- |
| 🧮 **Sieve of Eratosthenes (MPI)** | `mpicc`             | Ensure the `CC` variable in the `Makefile` is set to `mpicc`.    |
| 🧬 **Game of Life (Pthreads)**     | `gcc`               | Must link with `-pthread` (already handled by the Makefile).     |
| 🌌 **N-Body Simulation (Hybrid)**  | `mpicc` with OpenMP | Add `-fopenmp` to both `CXXFLAGS` and `LDFLAGS` in the Makefile. |

---

## 📘 License & Credits

These projects were developed for educational purposes as part of a **Parallel Computing** course.
All code is open for study, experimentation, and learning.

---

🧠 *“Parallelism is not just about dividing work — it’s about rethinking how work is done.”*