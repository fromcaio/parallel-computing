# Technical Report Writing Guide

## 1. Sequential Baseline

**Evidence**: Use the "Pure_MPI" (Rank=1) data from `benchmark_results.csv`.

**Discussion**: Discuss $O(N^2)$ complexity. The "InteractionsPerSec" metric in the CSV is your hardware-independent baseline.

## 2. Parallel Design

**Decomposition**: Explain the 1D block distribution used in `DomainDecomposition.cpp`.

**Communication**: Explain `MPI_Allgatherv`. Why gather all bodies? (Because every body exerts a force on every other body).

## 3. Evaluation and Scalability (The Core Section)

### Strong Scaling Plot

- Use the first plot from `analysis.ipynb`.
- **X-axis**: MPI Ranks
- **Y-axis**: Speedup
- **Analysis**: Does it follow the ideal line? Where does it diverge? (Likely at higher ranks due to `Allgatherv` overhead).

### Hybrid Comparison

- Use the second plot from `analysis.ipynb`.
- Compare **4 MPI × 1 Thread** vs **2 MPI × 2 Threads**.
- **Discussion**: If Pure MPI is faster, explain why (usually better cache locality for this specific problem size). If Hybrid is faster, explain why (reduced MPI communication overhead).

## 4. Conclusion

- Summarize the maximum speedup achieved.
- Discuss the trade-offs found in the notebook analysis.