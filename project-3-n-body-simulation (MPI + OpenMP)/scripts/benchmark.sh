#!/bin/bash

# Usage: ./scripts/benchmark.sh

# 1. Build project
cmake --build build -j

# 2. Setup Output CSV
RESULTS_FILE="benchmark_results.csv"
echo "Config,Ranks,Threads,Time,InteractionsPerSec" > $RESULTS_FILE

# 3. Generate Benchmark Input (N=5000 is good for scaling tests)
INPUT_FILE="benchmark_input.txt"
N_BODIES=5000
STEPS=20

echo "Generating input for N=$N_BODIES..."
python3 -c "
import random
print(f'$N_BODIES\n$STEPS\n0.1')
for _ in range($N_BODIES):
    print(f'1.0 {random.random()} {random.random()} 0.0 0.0')
" > $INPUT_FILE

# Helper function to run and log
run_test() {
    NAME=$1
    R=$2
    T=$3
    
    echo "Running: $NAME (Ranks=$R, Threads=$T)..."
    export OMP_NUM_THREADS=$T
    
    # Run, capture output
    # FIX: Added --map-by node:PE=$T to ensure rank gets T cores
    OUTPUT=$(mpirun -n $R --map-by node:PE=$T --bind-to core ./build/nbody_sim -i $INPUT_FILE -o /dev/null)
    
    # Parse Wall Time (Expects: "Wall Time  : 1.2345 s")
    # 'awk' splits by space: $1="Wall", $2="Time", $3=":", $4="1.2345"
    TIME=$(echo "$OUTPUT" | grep "Wall Time" | awk '{print $4}')
    
    # Parse Perf (Expects: "Performance: 1.23e+09 interactions/s")
    # 'awk' splits by space: $1="Performance:", $2="1.23e+09", $3="interactions/s"
    # FIX: Changed print $3 to print $2 to get the number
    PERF=$(echo "$OUTPUT" | grep "Performance" | awk '{print $2}')
    
    echo "$NAME,$R,$T,$TIME,$PERF" >> $RESULTS_FILE
}

# ---------------------------------------------------------
# Experiment 1: Strong Scaling (Pure MPI)
# ---------------------------------------------------------
run_test "Pure_MPI" 1 1
run_test "Pure_MPI" 2 1
run_test "Pure_MPI" 4 1
run_test "Pure_MPI" 8 1
run_test "Pure_MPI" 10 1

# ---------------------------------------------------------
# Experiment 2: Hybrid Comparison (Total 4 Cores)
# ---------------------------------------------------------
# Intermediate check
run_test "Hybrid_4c" 2 2
run_test "Pure_OMP_4c" 1 4

# ---------------------------------------------------------
# Experiment 3: Full Node Utilization (10 Cores)
# ---------------------------------------------------------
# Compare different splits of 10 cores
# 1. Pure MPI (10 Ranks, 1 Thread) -> Already run in Exp 1
# 2. Hybrid (2 Ranks, 5 Threads)
run_test "Hybrid_10c" 2 5
# 3. Hybrid (5 Ranks, 2 Threads)
run_test "Hybrid_10c" 5 2
# 4. Pure OMP (1 Rank, 10 Threads)
run_test "Pure_OMP_10c" 1 10

echo "---------------------------------------------------"
echo "Done. Results saved to $RESULTS_FILE"
echo "Open 'analysis.ipynb' to visualize results."