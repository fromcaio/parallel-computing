# 🧩 Parallel Computing Projects

A collection of **academic projects** developed for a *Parallel Computing* course.

Each implementation explores a different model of parallel programming, utilizing both **C** and **Modern C++**.

* 🚀 **MPI** (Message Passing Interface)
* 🧵 **Pthreads** (POSIX Threads)
* ⚡ **Hybrid** approach using **MPI + OpenMP**

---

## 🗂️ Repository Structure

Each project is contained in its own directory:

| Directory | Description | Language |
| --- | --- | --- |
| `project-1-sieve-mpi/` | Sieve of Eratosthenes using MPI | **C** |
| `project-2-game-of-life-pthreads/` | Conway's Game of Life using Pthreads | **C** |
| `project-3-n-body-hybrid/` | N-Body Simulation using hybrid MPI + OpenMP | **C++17** |

---

## 🧱 Project Folder Layouts

Because the projects use different languages and build systems, the directory structures differ.

### Projects 1 & 2 (C / Makefile)

Standard makefile-based structure for C projects.

```text
/bin/           # Compiled executables
/obj/           # Object and dependency files (.o, .d)
/src/           # Source code (.c)
/src/include/   # Header files (.h)
/doc/           # Documentation
Makefile        # Build configuration

```

### Project 3 (C++ / CMake)

Modern C++ project structure using CMake and presets.

```text
/build/         # CMake build artifacts (generated)
/src/           # Source code (.cpp)
/include/       # Header files (.hpp)
/scripts/       # Benchmarking scripts & Python analysis
/doc/           # Documentation and LaTeX reports
CMakeLists.txt  # CMake configuration
CMakePresets.json

```

---

## 💡 Project Summaries

<details>
<summary><b>🧮 Project 1 — Sieve of Eratosthenes (MPI)</b></summary>

**Objective:** Efficiently find all prime numbers up to a given integer `N` using the Sieve of Eratosthenes algorithm.

**Technology:** C / MPI

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

**Technology:** C / Pthreads

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

**Technology:** C++17 / MPI / OpenMP

**Parallel Model:** **Hybrid Data Decomposition**

#### Overview

1. **MPI (Distributed Memory):** Bodies are distributed among MPI processes. Each process updates positions and velocities for its subset.
2. The  force computation is parallelized using **all-to-all communication** (`MPI_Allgatherv`).
3. **OpenMP (Shared Memory):** Within each MPI process, OpenMP parallelizes local force calculations across all cores.

</details>

---

## ⚙️ Build & Run Instructions

### 🛠️ For Projects 1 & 2 (C)

1. **Enter the project folder:**
```bash
cd project-1-sieve-mpi/ 
# or 
cd project-2-game-of-life-pthreads/

```


2. **Compile:**
```bash
make

```


3. **Run:**
```bash
./bin/<executable_name>

```



### 🛠️ For Project 3 (C++)

This project uses **CMake (>= 3.20)**.

1. **Enter the project folder:**
```bash
cd project-3-n-body-hybrid/

```


2. **Configure and Build (using Presets):**
```bash
cmake --preset release
cmake --build build -j

```


3. **Run:**
```bash
mpirun -n 4 ./build/nbody_sim -i input.txt -o output.txt

```



---

## 🧰 Compiler Requirements

| Project | Compiler / Standard | Requirements |
| --- | --- | --- |
| 🧮 **Sieve of Eratosthenes** | `mpicc` (C99) | MPI implementation (OpenMPI/MPICH) |
| 🧬 **Game of Life** | `gcc` (C99) | Must link with `-pthread`. |
| 🌌 **N-Body Simulation** | `mpicxx` (C++17) | CMake 3.20+, OpenMP support. |

---

## 📘 License & Credits

These projects were developed for educational purposes as part of a **Parallel Computing** course.
All code is open for study, experimentation, and learning.

---

🧠 *“Parallelism is not just about dividing work — it’s about rethinking how work is done.”*