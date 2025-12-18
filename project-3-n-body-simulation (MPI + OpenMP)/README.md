Below is an **updated, clean revision of the README.md**, incorporating **all three requested changes** while keeping the tone and structure MIT-appropriate and professional:

* `/doc` explicitly included in the project structure
* `documentation.pdf` referenced and explained
* `analysis.ipynb` moved into `/scripts` and described accordingly

You can replace the README with this version directly.

---

# 🌌 Hybrid MPI + OpenMP N-Body Simulation

This project implements a **high-performance N-Body simulation** to model gravitational interactions between particles in **2D space**. It employs a **hybrid parallel programming model**, combining:

* **MPI** for distributed-memory parallelism (domain decomposition across ranks)
* **OpenMP** for shared-memory parallelism (thread-level acceleration within each rank)

The simulation computes forces using **Newton’s Law of Universal Gravitation** with **(O(N^2))** complexity and updates particle states using the **Symplectic Euler (semi-implicit) integrator**, which offers improved stability for orbital dynamics.

---

## 📂 Project Structure

```
project-root/
├── build/                      # Compiled binaries and artifacts
├── doc/                        # Documentation
│   ├── documentation.pdf       # Final technical report (submission-ready)
│   └── REPORT_GUIDE.md         # Mapping between code, experiments, and report sections
├── include/                    # Header files (architecture definitions)
│   └── nbody/                  # Library namespaces
├── src/                        # Source files (implementation)
│   ├── Body.cpp                # MPI Datatype definitions
│   ├── DomainDecomposition.cpp # MPI data distribution logic
│   ├── IO.cpp                  # File Input/Output
│   ├── Simulation.cpp          # Core physics engine
│   └── main.cpp                # Entry point
├── scripts/                    # Scripts and analysis tools
│   ├── benchmark.sh            # Automated scaling experiments
│   ├── run_hybrid.sh           # Helper for hybrid MPI + OpenMP execution
│   └── analysis.ipynb          # Jupyter Notebook for performance analysis
├── CMakeLists.txt              # Build system configuration
├── CMakePresets.json           # Standardized build presets
└── README.md
```

---

## ⚙️ Compilation

This project uses **CMake (minimum version 3.20)** and provides a **`CMakePresets.json`** file for standardized, reproducible builds.

### ✅ Recommended Build Method (Using Presets)

This is the **preferred and supported way** to build the project.

```bash
# or configure (Release – recommended for performance measurements)
cmake --preset release

# Build
cmake --build build -j
```

The executable `nbody_sim` will be generated in the `build/` directory defined by the preset.

---

### 🔧 Alternative Manual Build (Without Presets)

```bash
mkdir build
cd build

cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j
```

> ⚠️ Note: When not using presets, compiler flags and optimizations may differ from the recommended configuration.

---

## 🚀 Execution

The program accepts input and output paths via command-line arguments.

### 1. Basic Execution (MPI Only)

Run with **4 MPI ranks**, **1 OpenMP thread per rank**:

```bash
mpirun -n 4 ./build/nbody_sim -i input.txt -o output.txt
```

---

### 2. Hybrid Execution (MPI + OpenMP)

Run with **2 MPI ranks**, each using **4 OpenMP threads** (total of 8 cores):

```bash
export OMP_NUM_THREADS=4
mpirun -n 2 --bind-to socket ./build/nbody_sim -i input.txt -o output.txt
```

---

### 3. Automated Benchmarking

To run the full suite of scaling experiments (Sequential, Pure MPI, and Hybrid):

```bash
chmod +x scripts/benchmark.sh
./scripts/benchmark.sh
```

The script generates the raw timing and performance data used in the final analysis.

---

## 📤 Input & Output

### Input Format

```
<Number of Bodies (int)>
<Number of Steps (int)>
<Time Step dt (double)>
<mass> <pos_x> <pos_y> <vel_x> <vel_y>
... (one line per body)
```

---

### Output

* **Console output**:

  * Simulation progress
  * Wall-clock time
  * Interactions per second

* **File output**:

  * Final state of all bodies, written in the same format as the input file

**Example Console Output:**

```
========================================
 Hybrid N-Body Simulation 
========================================
 Bodies     : 5000
 Steps      : 20
 MPI Ranks  : 2
 OMP Threads: 4 per rank
----------------------------------------
 Progress: 100.0%
----------------------------------------
 Simulation Complete.
 Wall Time  : 1.2500 s
 Performance: 4.00e+08 interactions/s
========================================
```

---

## 🧠 Implementation Highlights

* **Language**: C++17
* **Parallel Model**: Hybrid MPI + OpenMP
* **Algorithm**: All-pairs force computation ((O(N^2)))
* **Integrator**: Symplectic Euler (semi-implicit)
* **Communication Pattern**:

  * `MPI_Allgatherv` to replicate particle positions at the start of each timestep
* **Optimizations**:

  * Cache-aligned data structures (`alignas(32)`)
  * Custom `MPI_Datatype` for efficient struct transmission
  * `schedule(static)` for OpenMP loop scheduling

---

## 📈 Performance Analysis & Documentation

### Technical Report (`/doc/documentation.pdf`)

The final **technical report** is provided as `documentation.pdf` inside the `/doc` folder.
It presents:

* Sequential baseline analysis
* Parallel design rationale
* Strong and hybrid scaling results
* Interpretation of performance behavior
* Limitations and future work

---

### Jupyter Notebook (`scripts/analysis.ipynb`)

A Jupyter Notebook is provided for **post-processing and visualization** of benchmark data generated by `scripts/benchmark.sh`.

The notebook:

* Loads timing and configuration data
* Computes speedup and efficiency
* Generates plots used in the final report
* Supports reproducible performance analysis

> The notebook does **not** run MPI/OpenMP code and does **not** reimplement the simulation.
> All parallel computation remains in C++.

---

## 🧾 License

This project is released **for educational purposes**.

**Author**: *Caio Reis*