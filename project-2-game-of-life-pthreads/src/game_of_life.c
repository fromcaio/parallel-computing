#include "include/game_of_life.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

static inline size_t cell_index(const Grid *grid, int row, int col) {
    return (size_t)row * (size_t)grid->cols + (size_t)col;
}

Grid allocate_grid(int rows, int cols) {
    Grid grid = {rows, cols, NULL};
    if (rows <= 0 || cols <= 0) {
        return grid;
    }

    grid.cells = calloc((size_t)rows * (size_t)cols, sizeof(unsigned char));
    return grid;
}

void free_grid(Grid *grid) {
    if (!grid) {
        return;
    }

    free(grid->cells);
    grid->cells = NULL;
    grid->rows = 0;
    grid->cols = 0;
}

static int validate_dimensions(int generations, int rows, int cols, int alive_count) {
    if (generations < 0) {
        fprintf(stderr, "Invalid input: number of generations must be non-negative\n");
        return -1;
    }

    if (rows <= 0 || cols <= 0) {
        fprintf(stderr, "Invalid input: matrix dimensions must be positive\n");
        return -1;
    }

    if (alive_count < 0) {
        fprintf(stderr, "Invalid input: alive cell count must be non-negative\n");
        return -1;
    }

    return 0;
}

int load_world_from_file(const char *path, int *generations_out, Grid *grid_out) {
    if (!path || !generations_out || !grid_out) {
        return -1;
    }

    FILE *file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Failed to open %s: %s\n", path, strerror(errno));
        return -1;
    }

    int generations = 0;
    int rows = 0;
    int cols = 0;
    int alive_count = 0;

    if (fscanf(file, "%d", &generations) != 1 ||
        fscanf(file, "%d %d", &rows, &cols) != 2 ||
        fscanf(file, "%d", &alive_count) != 1) {
        fprintf(stderr, "Invalid input format in %s\n", path);
        fclose(file);
        return -1;
    }

    if (validate_dimensions(generations, rows, cols, alive_count) != 0) {
        fclose(file);
        return -1;
    }

    Grid grid = allocate_grid(rows, cols);
    if (!grid.cells) {
        fprintf(stderr, "Failed to allocate matrix of size %dx%d\n", rows, cols);
        fclose(file);
        return -1;
    }

    for (int i = 0; i < alive_count; ++i) {
        int r = 0;
        int c = 0;
        if (fscanf(file, "%d %d", &r, &c) != 2) {
            fprintf(stderr, "Invalid alive cell entry at line %d\n", i + 4);
            free_grid(&grid);
            fclose(file);
            return -1;
        }

        if (r < 0 || r >= rows || c < 0 || c >= cols) {
            fprintf(stderr, "Alive cell coordinates out of bounds: (%d, %d)\n", r, c);
            free_grid(&grid);
            fclose(file);
            return -1;
        }

        grid.cells[cell_index(&grid, r, c)] = 1;
    }

    fclose(file);

    *generations_out = generations;
    *grid_out = grid;
    return 0;
}

int count_alive_cells(const Grid *grid) {
    if (!grid || !grid->cells) {
        return 0;
    }

    const size_t total = (size_t)grid->rows * (size_t)grid->cols;
    int alive = 0;

    for (size_t i = 0; i < total; ++i) {
        alive += grid->cells[i] ? 1 : 0;
    }

    return alive;
}

int write_world(FILE *out, int generations, const Grid *grid) {
    if (!out || !grid || !grid->cells) {
        return -1;
    }

    const int alive = count_alive_cells(grid);

    if (fprintf(out, "%d\n", generations) < 0 ||
        fprintf(out, "%d %d\n", grid->rows, grid->cols) < 0 ||
        fprintf(out, "%d\n", alive) < 0) {
        return -1;
    }

    for (int r = 0; r < grid->rows; ++r) {
        for (int c = 0; c < grid->cols; ++c) {
            if (grid->cells[cell_index(grid, r, c)]) {
                if (fprintf(out, "%d %d\n", r, c) < 0) {
                    return -1;
                }
            }
        }
    }

    return 0;
}

static unsigned char next_state(const Grid *grid, int row, int col) {
    int alive_neighbors = 0;

    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) {
                continue;
            }

            const int nr = row + dr;
            const int nc = col + dc;

            if (nr < 0 || nr >= grid->rows || nc < 0 || nc >= grid->cols) {
                continue;
            }

            alive_neighbors += grid->cells[cell_index(grid, nr, nc)] ? 1 : 0;
        }
    }

    const unsigned char is_alive = grid->cells[cell_index(grid, row, col)];

    if (is_alive && (alive_neighbors == 2 || alive_neighbors == 3)) {
        return 1;
    }

    if (!is_alive && alive_neighbors == 3) {
        return 1;
    }

    return 0;
}

void step_range(const Grid *current, Grid *next, int start_row, int end_row) {
    if (!current || !next || !current->cells || !next->cells) {
        return;
    }

    const int clamped_start = start_row < 0 ? 0 : start_row;
    const int clamped_end = end_row > current->rows ? current->rows : end_row;

    for (int r = clamped_start; r < clamped_end; ++r) {
        for (int c = 0; c < current->cols; ++c) {
            next->cells[cell_index(next, r, c)] = next_state(current, r, c);
        }
    }
}
