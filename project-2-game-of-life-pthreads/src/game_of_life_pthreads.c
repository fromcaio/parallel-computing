#include "include/game_of_life.h"

#include <errno.h>
#include <pthread.h>
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

typedef struct {
    int thread_id;
    int start_row;
    int end_row;
    int generations;
    Grid **current;
    Grid **next;
    pthread_barrier_t *compute_barrier;
    pthread_barrier_t *swap_barrier;
} WorkerArgs;

static void *worker(void *arg) {
    WorkerArgs *data = (WorkerArgs *)arg;

    for (int gen = 0; gen < data->generations; ++gen) {
        Grid *current = *data->current;
        Grid *next = *data->next;

        step_range(current, next, data->start_row, data->end_row);

        pthread_barrier_wait(data->compute_barrier);

        if (data->thread_id == 0) {
            Grid *tmp = *data->current;
            *data->current = *data->next;
            *data->next = tmp;
        }

        pthread_barrier_wait(data->swap_barrier);
    }

    return NULL;
}

static int create_workers(int thread_count, int generations, Grid **current, Grid **next) {
    pthread_t *threads = calloc((size_t)thread_count, sizeof(pthread_t));
    WorkerArgs *args = calloc((size_t)thread_count, sizeof(WorkerArgs));
    pthread_barrier_t compute_barrier;
    pthread_barrier_t swap_barrier;

    if (!threads || !args) {
        fprintf(stderr, "Failed to allocate thread metadata\n");
        free(threads);
        free(args);
        return -1;
    }

    if (pthread_barrier_init(&compute_barrier, NULL, (unsigned)thread_count) != 0 ||
        pthread_barrier_init(&swap_barrier, NULL, (unsigned)thread_count) != 0) {
        fprintf(stderr, "Failed to initialize thread barriers\n");
        free(threads);
        free(args);
        return -1;
    }

    const int rows = (*current)->rows;
    const int base_rows = rows / thread_count;
    int remainder = rows % thread_count;
    int start = 0;

    for (int i = 0; i < thread_count; ++i) {
        int count = base_rows + (remainder > 0 ? 1 : 0);
        remainder = remainder > 0 ? remainder - 1 : 0;

        args[i].thread_id = i;
        args[i].start_row = start;
        args[i].end_row = start + count;
        args[i].generations = generations;
        args[i].current = current;
        args[i].next = next;
        args[i].compute_barrier = &compute_barrier;
        args[i].swap_barrier = &swap_barrier;

        start += count;

        if (pthread_create(&threads[i], NULL, worker, &args[i]) != 0) {
            fprintf(stderr, "Failed to create thread %d\n", i);
            for (int j = 0; j < i; ++j) {
                pthread_join(threads[j], NULL);
            }
            pthread_barrier_destroy(&compute_barrier);
            pthread_barrier_destroy(&swap_barrier);
            free(threads);
            free(args);
            return -1;
        }
    }

    for (int i = 0; i < thread_count; ++i) {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&compute_barrier);
    pthread_barrier_destroy(&swap_barrier);

    free(threads);
    free(args);
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input_file> <num_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *input_path = argv[1];
    const int thread_count = atoi(argv[2]);

    if (thread_count <= 0) {
        fprintf(stderr, "Number of threads must be a positive integer\n");
        return EXIT_FAILURE;
    }

    int generations = 0;
    Grid world = {0};

    if (load_world_from_file(input_path, &generations, &world) != 0) {
        return EXIT_FAILURE;
    }

    Grid buffer = allocate_grid(world.rows, world.cols);
    if (!buffer.cells) {
        fprintf(stderr, "Failed to allocate buffer for next generation\n");
        free_grid(&world);
        return EXIT_FAILURE;
    }

    Grid *current = &world;
    Grid *next = &buffer;

    struct timespec start_time = {0};
    struct timespec end_time = {0};

    clock_gettime(CLOCK_MONOTONIC, &start_time);

    if (create_workers(thread_count, generations, &current, &next) != 0) {
        free_grid(&world);
        free_grid(&buffer);
        return EXIT_FAILURE;
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    if (mkdir("output", 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "Failed to create output directory: %s\n", strerror(errno));
        free_grid(current);
        free_grid(next);
        return EXIT_FAILURE;
    }

    char output_path[160];
    snprintf(output_path, sizeof(output_path), "output/game_of_life_threads_%dt_%dx%d_%dgen.txt", thread_count, current->rows, current->cols, generations);

    FILE *out = fopen(output_path, "w");
    if (!out) {
        fprintf(stderr, "Failed to open %s for writing: %s\n", output_path, strerror(errno));
        free_grid(current);
        free_grid(next);
        return EXIT_FAILURE;
    }

    if (write_world(out, generations, current) != 0) {
        fprintf(stderr, "Failed to write final world to %s\n", output_path);
        fclose(out);
        free_grid(current);
        free_grid(next);
        return EXIT_FAILURE;
    }

    fclose(out);

    const double elapsed = elapsed_seconds(&start_time, &end_time);
    printf("[Threads] Using %d threads\n", thread_count);
    printf("[Threads] Execution time: %.6f seconds\n", elapsed);
    printf("[Threads] Output written to %s\n", output_path);

    free_grid(current);
    free_grid(next);

    return EXIT_SUCCESS;
}
