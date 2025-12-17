#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "sieve.h"

enum
{
    TAG_RANGE = 100,
    TAG_COUNT = 101,
    TAG_DATA = 102
};

static void run_master_process(int world_size, long long upper_bound);
static void run_slave_process(int world_rank);

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int world_size = 0;
    int world_rank = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    long long upper_bound = parse_upper_bound(argc, argv, world_rank);
    if (upper_bound < 2)
    {
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    if (world_rank == 0)
    {
        run_master_process(world_size, upper_bound);
    }
    else
    {
        run_slave_process(world_rank);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}

static void run_master_process(int world_size, long long upper_bound)
{
    double start_time = MPI_Wtime();
    printf("[Master] Using %d processes\n", world_size);

    int base_count = 0;
    long long *base_primes = generate_base_primes(upper_bound, &base_count);

    MPI_Bcast(&base_count, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (base_count > 0)
    {
        MPI_Bcast(base_primes, base_count, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
    }

    for (int dest = 1; dest < world_size; ++dest)
    {
        long long range[2] = {0, -1};
        compute_segment_bounds(upper_bound, world_size, dest, &range[0], &range[1]);
        MPI_Send(range, 2, MPI_LONG_LONG, dest, TAG_RANGE, MPI_COMM_WORLD);
    }

    long long local_start = 0;
    long long local_end = -1;
    compute_segment_bounds(upper_bound, world_size, 0, &local_start, &local_end);

    long long *local_primes = NULL;
    int local_count = 0;
    sieve_segment(local_start, local_end, base_primes, base_count, &local_primes, &local_count);

    int all_capacity = base_count + local_count;
    if (all_capacity < 16)
    {
        all_capacity = 16;
    }

    long long *all_primes = (long long *)malloc((size_t)all_capacity * sizeof(long long));
    if (all_primes == NULL)
    {
        fprintf(stderr, "Rank 0: Failed to allocate output buffer\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    int all_count = 0;
    append_values(&all_primes, &all_capacity, &all_count, base_primes, base_count);
    append_values(&all_primes, &all_capacity, &all_count, local_primes, local_count);
    free(local_primes);

    for (int source = 1; source < world_size; ++source)
    {
        int incoming_count = 0;
        MPI_Recv(&incoming_count, 1, MPI_INT, source, TAG_COUNT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (incoming_count <= 0)
        {
            continue;
        }

        long long *buffer = (long long *)malloc((size_t)incoming_count * sizeof(long long));
        if (buffer == NULL)
        {
            fprintf(stderr, "Rank 0: Failed to allocate receive buffer for %d primes\n", incoming_count);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        MPI_Recv(buffer, incoming_count, MPI_LONG_LONG, source, TAG_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        append_values(&all_primes, &all_capacity, &all_count, buffer, incoming_count);
        free(buffer);
    }

    if (all_count > 1)
    {
        qsort(all_primes, (size_t)all_count, sizeof(long long), compare_long_long);
        int unique_index = 0;
        for (int i = 1; i < all_count; ++i)
        {
            if (all_primes[i] != all_primes[unique_index])
            {
                all_primes[++unique_index] = all_primes[i];
            }
        }
        all_count = unique_index + 1;
    }

    double end_time = MPI_Wtime();

    int valid_count = 0;
    for (int i = 0; i < all_count; ++i)
    {
        if (all_primes[i] >= 2 && all_primes[i] <= upper_bound)
        {
            all_primes[valid_count++] = all_primes[i];
        }
    }
    all_count = valid_count;

    printf("[Master] Computed %d primes up to N = %lld\n", all_count, upper_bound);
    printf("[Master] Execution time (computation only): %f seconds\n", end_time - start_time);
    write_primes_to_file("primes.txt", all_primes, all_count);
    printf("[Master] Output written to primes.txt\n");

    free(all_primes);
    free(base_primes);
}

static void run_slave_process(int world_rank)
{
    int base_count = 0;
    MPI_Bcast(&base_count, 1, MPI_INT, 0, MPI_COMM_WORLD);

    long long *base_primes = NULL;
    if (base_count > 0)
    {
        base_primes = (long long *)malloc((size_t)base_count * sizeof(long long));
        if (base_primes == NULL)
        {
            fprintf(stderr, "Rank %d: Failed to allocate base primes buffer\n", world_rank);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
    }
    MPI_Bcast(base_primes, base_count, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

    long long range[2] = {0, -1};
    MPI_Recv(range, 2, MPI_LONG_LONG, 0, TAG_RANGE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    long long local_start = range[0];
    long long local_end = range[1];

    long long *local_primes = NULL;
    int local_count = 0;
    sieve_segment(local_start, local_end, base_primes, base_count, &local_primes, &local_count);

    MPI_Send(&local_count, 1, MPI_INT, 0, TAG_COUNT, MPI_COMM_WORLD);
    if (local_count > 0)
    {
        MPI_Send(local_primes, local_count, MPI_LONG_LONG, 0, TAG_DATA, MPI_COMM_WORLD);
    }

    free(local_primes);
    free(base_primes);
}
