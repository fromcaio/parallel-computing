#include "include/game_of_life.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

static double elapsed_seconds(const struct timespec *start, const struct timespec *end) {
    const long sec = end->tv_sec - start->tv_sec;
    const long nsec = end->tv_nsec - start->tv_nsec;
    return (double)sec + (double)nsec / 1e9;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *input_path = argv[1];

    int generations = 0;
    Grid current = {0};

    if (load_world_from_file(input_path, &generations, &current) != 0) {
        return EXIT_FAILURE;
    }

    Grid next = allocate_grid(current.rows, current.cols);
    if (!next.cells) {
        fprintf(stderr, "Failed to allocate buffer for next generation\n");
        free_grid(&current);
        return EXIT_FAILURE;
    }

    struct timespec start_time = {0};
    struct timespec end_time = {0};

    clock_gettime(CLOCK_MONOTONIC, &start_time);

    for (int gen = 0; gen < generations; ++gen) {
        step_range(&current, &next, 0, current.rows);

        unsigned char *tmp = current.cells;
        current.cells = next.cells;
        next.cells = tmp;
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    if (mkdir("output", 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "Failed to create output directory: %s\n", strerror(errno));
        free_grid(&current);
        free_grid(&next);
        return EXIT_FAILURE;
    }

    char output_path[160];
    snprintf(output_path, sizeof(output_path), "output/game_of_life_seq_%dx%d_%dgen.txt", current.rows, current.cols, generations);

    FILE *out = fopen(output_path, "w");
    if (!out) {
        fprintf(stderr, "Failed to open %s for writing: %s\n", output_path, strerror(errno));
        free_grid(&current);
        free_grid(&next);
        return EXIT_FAILURE;
    }

    if (write_world(out, generations, &current) != 0) {
        fprintf(stderr, "Failed to write final world to %s\n", output_path);
        fclose(out);
        free_grid(&current);
        free_grid(&next);
        return EXIT_FAILURE;
    }

    fclose(out);

    const double elapsed = elapsed_seconds(&start_time, &end_time);
    const long peak_kb = get_peak_rss_kb();
    printf("[Sequential] Execution time: %.6f seconds\n", elapsed);
    if (peak_kb >= 0) {
        printf("[Sequential] Peak memory: %ld KB\n", peak_kb);
    }
    printf("[Sequential] Output written to %s\n", output_path);

    free_grid(&current);
    free_grid(&next);

    return EXIT_SUCCESS;
}
