#!/bin/bash

# Usage: ./scripts/run_hybrid.sh <N_RANKS> <OMP_THREADS> <INPUT_FILE>
# Example: ./scripts/run_hybrid.sh 2 4 input.txt

if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <N_RANKS> <OMP_THREADS> <INPUT_FILE>"
    exit 1
fi

RANKS=$1
THREADS=$2
INPUT=$3

# 1. Set OpenMP Environment
export OMP_NUM_THREADS=$THREADS
export OMP_PROC_BIND=true
export OMP_PLACES=threads

# 2. Set MPI Binding flags (OpenMPI specific, but common)
# --map-by socket:pe=$THREADS -> Places ranks on sockets, spacing them by $THREADS processing elements
# This ensures Rank 0 gets cores 0-(N-1), Rank 1 gets N-(2N-1), etc.

echo "Running with $RANKS MPI Ranks x $THREADS OpenMP Threads..."
echo "OMP_NUM_THREADS=$OMP_NUM_THREADS"
echo "OMP_PROC_BIND=$OMP_PROC_BIND"

# Note: The exact mpirun flags depend heavily on the cluster scheduler (Slurm, PBS).
# This is a generic "best effort" for a local machine or standard OpenMPI install.

mpirun -n $RANKS \
    --map-by node:PE=$THREADS \
    --bind-to core \
    ./build/nbody_sim -i $INPUT -o output.txt

# If the above fails on your specific system, try the simpler:
# mpirun -n $RANKS ./build/nbody_sim -i $INPUT -o output.txt