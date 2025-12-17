# 🧬 Conway's Game of Life — Sequential & Pthreads

Parallel and sequential implementations of Conway's Game of Life in C. The parallel version uses POSIX threads (barriers + domain decomposition) as a shared-memory counterpart to the MPI sieve project.

---

## 📂 Project Structure

```
project-2-game-of-life-pthreads/
├── Makefile
├── src/
│   ├── game_of_life.c          # Shared core (I/O + step logic)
│   ├── game_of_life_sequential.c
│   ├── game_of_life_pthreads.c
│   └── include/game_of_life.h
├── samples/                    # Example input patterns
└── output/                     # Run outputs (created at runtime)
```

Input format (same for both builds):

```
<N_GEN>
<ROWS> <COLS>
<N_ALIVE>
<r> <c>   # one line per initially alive cell
...
```

Outputs follow the same format and are written to `output/`.

---

## ⚙️ Compilation

From `project-2-game-of-life-pthreads/`:

```bash
# Parallel (default)
make

# Sequential
make sequential

# Clean
make clean
```

Artifacts land in `bin/`:

- Parallel: `bin/game_of_life_pthread`
- Sequential: `bin/game_of_life_seq`

---

## 🚀 Execution

```bash
# Parallel: <input_file> <num_threads>
./bin/game_of_life_pthread samples/glider.txt 4

# Sequential: <input_file>
./bin/game_of_life_seq samples/blinker.txt
```

Both binaries log timing (simulation only, excludes file write) and the output path. Filenames include grid size and generations, e.g.:

- `[Threads] Using 5 threads`
- `[Threads] Execution time: 14.146448 seconds`
- `[Threads] Peak memory: 196656 KB`
- `[Threads] Output written to output/game_of_life_threads_5t_10000x10000_8gen.txt`

Sequential uses `output/game_of_life_seq_<rows>x<cols>_<gen>gen.txt`.

---

## 🧠 Implementation Highlights

- Language: C11, `-pthread` for the parallel build.
- Work split: row-block domain decomposition across threads.
- Synchronization: two barriers per generation (compute + swap) to keep grids consistent.
- Data layout: contiguous `rows x cols` byte grid for cache-friendly traversal.
- I/O: shared helpers handle parsing, validation, and writing in the agreed format.

---

## 📊 Memory Usage Measurement (Peak RSS)
This project reports peak physical memory usage (RSS) using:

```c
getrusage(RUSAGE_SELF).ru_maxrss
```

This value represents the **maximum resident set size** of the process:
the amount of RAM actually used during execution (not virtual memory).

### What is included in RSS?

* Both grids (current + next)
* All heap allocations
* All stack memory **actually used** by all threads
* Shared libraries loaded by the program
* Loaded ELF binary sections

### What is *not* included?

* Untouched portions of pthread stacks (Linux reserves ~8 MB per thread virtually, but only pages that are written count toward RSS)

### Why do sequential and parallel versions use similar memory?

Even though each thread conceptually has an 8 MB stack, Linux allocates stacks **lazily**. A page is only committed to RAM when a thread actually touches it. Because the worker threads in this project use only small local variables and no recursion, each thread typically touches just a few kilobytes of stack.

Therefore, the dominant memory usage is:

```
2 × (rows × cols) bytes   # the two grids (current and next)
+ libc + binary + small stacks
≈ total RSS
```

For example:

* A `1000×1000` grid uses ~2 MB for both grids → peak RSS ≈ 7–8 MB
* A `10000×10000` grid uses ~190 MB for both grids → peak RSS ≈ 190–200 MB
* Thread count has almost no impact on RSS unless stack usage grows deep

This matches the observed results in all runs.

### 🔧 Reducing Memory Usage

The current implementation represents each cell using **1 byte (`unsigned char`)**, where the value is:

```
0 = dead
1 = alive
```

However, the Game of Life only requires **one bit per cell**, since each cell is binary.
By packing 8 cells into a single byte, memory usage can be reduced by a factor of **8×**.

#### Why use bit-packing?

* **Lower memory footprint:**
  A `10000×10000` grid goes from ~100 MB per grid to ~12.5 MB.
* **Better cache behavior:**
  More cells fit in L1/L2 cache, reducing cache misses and improving performance.
* **Scales much better for very large grids** (e.g., 20k×20k or 50k×50k).

#### What is required to implement it?

Bit-packing changes how each cell is accessed:

* Reading a cell becomes:

  ```c
  (grid[i >> 3] >> (i & 7)) & 1
  ```
* Writing a cell becomes:

  ```c
  grid[i >> 3] |=  (1 << (i & 7));   // set bit  
  grid[i >> 3] &= ~(1 << (i & 7));   // clear bit
  ```

Neighbor-counting needs to be adapted to operate on bit-packed indices,
but the overall algorithm structure remains the same.

While this optimization adds complexity, it provides significant **memory and performance benefits**,
especially for large simulations or many generations.

## 🧪 Samples

A wide set of ready-to-use input files is available in the `samples/` directory.
They cover small debugging patterns, medium-sized configurations for algorithm testing, and large/XL grids for performance and memory benchmarking.

### **Small patterns (5×5 to 15×15)**

Ideal for correctness validation and stepping through generations manually.

* `small_blinker_5x5_4gen.txt` — blinker oscillator
* `small_block_5x5_stable.txt` — stable 2×2 block
* `small_beehive_6x6_stable.txt` — still-life
* `small_glider_10x10_8gen.txt` — classic glider
* `small_toad_6x6_4gen.txt` — period-2 oscillator
* `small_pulsar_15x15_3gen.txt` — larger oscillator

### **Medium patterns (80×80 to 100×100)**

Useful for testing scalability, correctness, and domain decomposition.

* `medium_glider_gun_100x100_30gen.txt` — Gosper glider gun
* `medium_rpentomino_80x80_110gen.txt` — chaotic R-pentomino evolution
* `medium_random_100x100_50pct_50gen.txt` — 50% random initialization
* `medium_cross_100x100_40gen.txt` — symmetric cross-shaped pattern

### **Large patterns (500×500 to 1500×1500)**

Designed to evaluate cache behavior, memory bandwidth, and multi-thread performance.

* `large_random_500x500_20pct_100gen.txt`
* `large_random_1000x1000_10pct_200gen.txt`
* `large_glider_field_1500x1500_120gen.txt`
* `large_dense_1000x1000_70pct_50gen.txt`

### **Extra-large patterns (5000×5000 to 10000×10000)**

Stress tests for peak memory usage, swap behavior, and throughput.

* `xl_random_5000x5000_10pct_50gen.txt`
* `xl_random_10000x10000_5pct_8gen.txt`

### **Generating Samples Programmatically**

All samples listed above can be generated automatically using:

```
python3 tools/generate_samples.py
```

This script creates the full dataset inside the `samples/` directory and can be re-run safely (files are overwritten).
It supports deterministic randomness for reproducible results.

 