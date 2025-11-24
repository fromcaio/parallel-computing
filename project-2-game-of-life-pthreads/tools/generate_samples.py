#!/usr/bin/env python3
import os
import random
from pathlib import Path

SAMPLES_DIR = Path("samples")
SAMPLES_DIR.mkdir(exist_ok=True)

# ------------------------------------------------------------
# Helper functions
# ------------------------------------------------------------

def write_sample(filename, rows, cols, generations, alive_cells):
    """Write a sample file in the required format."""
    path = SAMPLES_DIR / filename
    with open(path, "w") as f:
        f.write(f"{generations}\n")
        f.write(f"{rows} {cols}\n")
        f.write(f"{len(alive_cells)}\n")
        for r, c in alive_cells:
            f.write(f"{r} {c}\n")
    print(f"[OK] Generated {path}")


def center_pattern(rows, cols, pattern):
    """Center a pattern (list of (r,c)) inside a rows x cols grid."""
    if not pattern:
        return []

    min_r = min(r for r, _ in pattern)
    max_r = max(r for r, _ in pattern)
    min_c = min(c for _, c in pattern)
    max_c = max(c for _, c in pattern)

    pat_h = max_r - min_r + 1
    pat_w = max_c - min_c + 1

    offset_r = (rows - pat_h) // 2 - min_r
    offset_c = (cols - pat_w) // 2 - min_c

    return [(r + offset_r, c + offset_c) for r, c in pattern]


def random_pattern(rows, cols, density):
    """density = percentage of alive cells (0.0 to 1.0)."""
    alive = []
    for r in range(rows):
        for c in range(cols):
            if random.random() < density:
                alive.append((r, c))
    return alive


# ------------------------------------------------------------
# Predefined classic patterns
# ------------------------------------------------------------

def blinker():
    # Horizontal 3-cell blinker
    return [(0,0), (0,1), (0,2)]

def block():
    return [(0,0), (0,1), (1,0), (1,1)]

def beehive():
    return [(0,1), (0,2), (1,0), (1,3), (2,1), (2,2)]

def toad():
    # Period-2 oscillator
    return [(0,1),(0,2),(0,3),(1,0),(1,1),(1,2)]

def pulsar():
    # A classic 15x15 pulsar pattern
    p = []
    base = [
        (2,4),(2,5),(2,6),(2,10),(2,11),(2,12),
        (7,4),(7,5),(7,6),(7,10),(7,11),(7,12),
        (9,4),(9,5),(9,6),(9,10),(9,11),(9,12),
        (4,2),(5,2),(6,2),(4,7),(5,7),(6,7),(4,9),(5,9),(6,9),(4,14),(5,14),(6,14),
        (10,2),(11,2),(12,2),(10,7),(11,7),(12,7),(10,9),(11,9),(12,9),(10,14),(11,14),(12,14)
    ]
    return base

def glider():
    return [(0,1),(1,2),(2,0),(2,1),(2,2)]

def glider_gun():
    # Gosper Glider Gun (36x9 footprint)
    gun = [
        (5,1),(5,2),(6,1),(6,2),
        (3,13),(3,14),(4,12),(4,16),(5,11),(5,17),(6,11),(6,15),(6,17),(6,18),
        (7,11),(7,17),(8,12),(8,16),(9,13),(9,14),
        (1,25),(2,23),(2,25),(3,21),(3,22),(4,21),(4,22),(5,21),(5,22),
        (6,23),(6,25),(7,25),
        (3,35),(3,36),(4,35),(4,36)
    ]
    return gun


# ------------------------------------------------------------
# Generate all samples
# ------------------------------------------------------------

def main():
    random.seed(42)  # reproducible samples

    # ---------------- Small patterns ----------------
    write_sample(
        "small_blinker_5x5_4gen.txt",
        5, 5, 4,
        center_pattern(5, 5, blinker())
    )

    write_sample(
        "small_block_5x5_stable.txt",
        5, 5, 1,
        center_pattern(5, 5, block())
    )

    write_sample(
        "small_beehive_6x6_stable.txt",
        6, 6, 1,
        center_pattern(6, 6, beehive())
    )

    write_sample(
        "small_glider_10x10_8gen.txt",
        10, 10, 8,
        center_pattern(10, 10, glider())
    )

    write_sample(
        "small_toad_6x6_4gen.txt",
        6, 6, 4,
        center_pattern(6, 6, toad())
    )

    write_sample(
        "small_pulsar_15x15_3gen.txt",
        15, 15, 3,
        center_pattern(15, 15, pulsar())
    )

    # ---------------- Medium patterns ----------------
    write_sample(
        "medium_glider_gun_100x100_30gen.txt",
        100, 100, 30,
        center_pattern(100, 100, glider_gun())
    )

    write_sample(
        "medium_rpentomino_80x80_110gen.txt",
        80, 80, 110,
        center_pattern(80, 80, [(0,1),(1,0),(1,1),(1,2),(2,0)])  # R-pentomino
    )

    write_sample(
        "medium_random_100x100_50pct_50gen.txt",
        100, 100, 50,
        random_pattern(100, 100, 0.50)
    )

    write_sample(
        "medium_cross_100x100_40gen.txt",
        100, 100, 40,
        center_pattern(100, 100,
            [(i,50) for i in range(30,70)] +
            [(50,j) for j in range(30,70)]
        )
    )

    # ---------------- Large patterns ----------------
    write_sample(
        "large_random_500x500_20pct_100gen.txt",
        500, 500, 100,
        random_pattern(500, 500, 0.20)
    )

    write_sample(
        "large_random_1000x1000_10pct_200gen.txt",
        1000, 1000, 200,
        random_pattern(1000, 1000, 0.10)
    )

    write_sample(
        "large_glider_field_1500x1500_120gen.txt",
        1500, 1500, 120,
        center_pattern(1500, 1500, glider())
    )

    write_sample(
        "large_dense_1000x1000_70pct_50gen.txt",
        1000, 1000, 50,
        random_pattern(1000, 1000, 0.70)
    )

    # ---------------- Extra-large patterns ----------------
    write_sample(
        "xl_random_5000x5000_10pct_50gen.txt",
        5000, 5000, 50,
        random_pattern(5000, 5000, 0.10)
    )

    write_sample(
        "xl_random_10000x10000_5pct_8gen.txt",
        10000, 10000, 8,
        random_pattern(10000, 10000, 0.05)
    )


main()