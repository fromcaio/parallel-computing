#ifndef GAME_OF_LIFE_H
#define GAME_OF_LIFE_H

#include <stdio.h>
#include <stddef.h>

typedef struct {
    int rows;
    int cols;
    unsigned char *cells;
} Grid;

Grid allocate_grid(int rows, int cols);
void free_grid(Grid *grid);

int load_world_from_file(const char *path, int *generations_out, Grid *grid_out);
int write_world(FILE *out, int generations, const Grid *grid);
int count_alive_cells(const Grid *grid);

void step_range(const Grid *current, Grid *next, int start_row, int end_row);

#endif
