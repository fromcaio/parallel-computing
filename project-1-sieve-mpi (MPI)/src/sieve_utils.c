#include "sieve.h"

#include <errno.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

long long parse_upper_bound(int argc, char **argv, int rank)
{
    if (argc < 2)
    {
        if (rank == 0)
        {
            fprintf(stderr, "Usage: %s N\n", argv[0]);
        }
        return -1;
    }

    errno = 0;
    char *endptr = NULL;
    long long value = strtoll(argv[1], &endptr, 10);
    if (errno != 0 || endptr == argv[1] || *endptr != '\0' || value < 2)
    {
        if (rank == 0)
        {
            fprintf(stderr, "Invalid upper bound '%s'. Please provide an integer >= 2.\n", argv[1]);
        }
        return -1;
    }
    return value;
}

void append_values(long long **array, int *capacity, int *count, const long long *values, int values_count)
{
    if (values_count <= 0)
    {
        return;
    }

    if (*count + values_count > *capacity)
    {
        int new_capacity = *capacity;
        while (new_capacity < *count + values_count)
        {
            new_capacity *= 2;
        }

        long long *resized = (long long *)realloc(*array, (size_t)new_capacity * sizeof(long long));
        if (resized == NULL)
        {
            fprintf(stderr, "Rank 0: Failed to grow output buffer\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        *array = resized;
        *capacity = new_capacity;
    }

    for (int i = 0; i < values_count; ++i)
    {
        (*array)[*count + i] = values[i];
    }
    *count += values_count;
}

int compare_long_long(const void *lhs, const void *rhs)
{
    long long a = *(const long long *)lhs;
    long long b = *(const long long *)rhs;

    if (a < b)
    {
        return -1;
    }
    if (a > b)
    {
        return 1;
    }
    return 0;
}

void write_primes_to_file(const char *filename, const long long *primes, int count)
{
    FILE *output = fopen(filename, "w");
    if (output == NULL)
    {
        fprintf(stderr, "Rank 0: Failed to open output file '%s'\n", filename);
        return;
    }

    for (int i = 0; i < count; ++i)
    {
        fprintf(output, "%lld\n", primes[i]);
    }

    fclose(output);
}
