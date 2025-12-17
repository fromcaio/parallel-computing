# 🧮 Parallel and Sequential Sieve of Eratosthenes (MPI in C)

This project implements the **Sieve of Eratosthenes** algorithm to find all prime numbers up to a given limit **N**, using both **sequential** and **parallel (MPI)** approaches.

The **parallel version** follows the **Master–Slave model**, distributing the workload across multiple processes using **MPI**.  
The **sequential version** serves as a baseline for performance comparison and execution time analysis.

---

## 📂 Project Structure

```bash
project-root/
├── Makefile
├── doc/                       # Documentation and reports
├── bin/                       # Final binaries
├── obj/                       # Object (.o) and dependency (.d) files
├── lib/                       # External libraries (not used)
└── src/
    ├── sieve_parallel.c       # Parallel MPI implementation (Master–Slave model)
    ├── sieve_sequential.c     # Sequential baseline version
    └── include/               # Header files
````

---

## ⚙️ Compilation

Use the provided **Makefile** to build the executables.

```bash
# Build parallel MPI version (default target)
make

# Build sequential version
make sequential
```

All compiled binaries are placed inside the `bin/` directory.

---

## 🚀 Execution

Each program accepts a single argument **N**, representing the upper bound for prime computation.

```bash
# Sequential version
./bin/sieve_seq 1000000

# Parallel version (example with 4 processes)
mpirun -np 4 ./bin/program 1000000
```

---

## 📤 Output

* The **parallel version** generates a file named `primes.txt`,
  containing all prime numbers between `2` and `N` in ascending order.
* The **sequential version** generates a file named `primes-sequential.txt`,
  containing the same list of primes.
* Both versions print runtime information to the terminal, showing the total **execution time** in seconds.

Example console output:

```
[Sequential] Computed 78498 primes up to N = 1000000
Execution time: 0.127 seconds
```

---

## 🧠 Implementation Highlights

* **Language:** C (`-std=c11`)
* **Parallel model:** Master–Slave using MPI
* **Base primes:** Computed sequentially up to √N and broadcasted to all processes
* **Work division:** Each process sieves a subrange of `[2, N]`
* **Result gathering:** Master collects and merges all local results
* **Performance metric:** Execution time comparison between sequential and parallel runs

---

## 📈 Performance Evaluation (Optional)

Performance can be evaluated by comparing the **sequential** and **parallel** runtimes to compute:

* **Speedup (S):** `S = Ts / Tp`
* **Efficiency (E):** `E = S / p` (where *p* is the number of processes)

Example run:

```bash
mpirun -np 4 ./bin/program 10000000
```

---

## 🧾 License

This project is released for educational purposes under the **MIT License**.

---

**Author:** Caio Reis