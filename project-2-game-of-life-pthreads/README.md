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

- `[Threads] Using 4 threads`
- `[Threads] Execution time: 0.012345 seconds`
- `[Threads] Output written to output/game_of_life_threads_4t_10x10_8gen.txt`

Sequential uses `output/game_of_life_seq_<rows>x<cols>_<gen>gen.txt`.

---

## 🧠 Implementation Highlights

- Language: C11, `-pthread` for the parallel build.
- Work split: row-block domain decomposition across threads.
- Synchronization: two barriers per generation (compute + swap) to keep grids consistent.
- Data layout: contiguous `rows x cols` byte grid for cache-friendly traversal.
- I/O: shared helpers handle parsing, validation, and writing in the agreed format.

---

## 🧪 Samples

Ready-to-use inputs in `samples/`:

- `blinker.txt` (5x5, 4 generations)
- `block.txt` (5x5, stable)
- `glider.txt` (10x10, 8 generations)

Use them directly with the execution commands metioned above; outputs will be written to `output/`.