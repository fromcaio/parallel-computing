#include "sieve.h"

#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

long long integer_sqrt(long long value)
{
    if (value <= 0)
    {
        return 0;
    }

    long long low = 1;
    long long high = value;
    long long result = 0;

    while (low <= high)
    {
        long long mid = low + (high - low) / 2;
        if (mid <= value / mid)
        {
            result = mid;
            low = mid + 1;
        }
        else
        {
            high = mid - 1;
        }
    }

    return result;
}

long long *generate_base_primes(long long n, int *count_out)
{
    long long limit = integer_sqrt(n);
    if (limit < 2)
    {
        *count_out = 0;
        return NULL;
    }

    bool *is_composite = (bool *)calloc((size_t)limit + 1, sizeof(bool));
    if (is_composite == NULL)
    {
        fprintf(stderr, "Rank 0: Failed to allocate base sieve array\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    for (long long candidate = 2; candidate * candidate <= limit; ++candidate)
    {
        if (!is_composite[candidate])
        {
            for (long long multiple = candidate * candidate; multiple <= limit; multiple += candidate)
            {
                is_composite[multiple] = true;
            }
        }
    }

    int count = 0;
    for (long long value = 2; value <= limit; ++value)
    {
        if (!is_composite[value])
        {
            ++count;
        }
    }

    long long *primes = NULL;
    if (count > 0)
    {
        primes = (long long *)malloc((size_t)count * sizeof(long long));
        if (primes == NULL)
        {
            free(is_composite);
            fprintf(stderr, "Rank 0: Failed to allocate base primes array\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        int index = 0;
        for (long long value = 2; value <= limit; ++value)
        {
            if (!is_composite[value])
            {
                primes[index++] = value;
            }
        }
    }

    free(is_composite);
    *count_out = count;
    return primes;
}

void compute_segment_bounds(long long n, int world_size, int rank, long long *start, long long *end)
{
    if (n < 2)
    {
        *start = 0;
        *end = -1;
        return;
    }

    long long total_numbers = n - 1;
    long long base = total_numbers / world_size;
    long long remainder = total_numbers % world_size;

    long long offset = rank * base + (rank < remainder ? rank : remainder);
    long long length = base + (rank < remainder ? 1 : 0);

    *start = 2 + offset;
    *end = length > 0 ? (*start + length - 1) : (*start - 1);
}

void sieve_segment(long long start, long long end, const long long *base_primes, int base_count,
                   long long **primes_out, int *count_out)
{
    if (start > end || end < 2)
    {
        *primes_out = NULL;
        *count_out = 0;
        return;
    }

    if (start < 2)
    {
        start = 2;
    }

    size_t window_size = (size_t)(end - start + 1);
    bool *is_composite = (bool *)calloc(window_size, sizeof(bool));
    if (is_composite == NULL)
    {
        fprintf(stderr, "Failed to allocate segment sieve array\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    for (int i = 0; i < base_count; ++i)
    {
        long long prime = base_primes[i];
        long long first_multiple = (start + prime - 1) / prime * prime;
        long long prime_square = prime * prime;
        if (prime_square > first_multiple)
        {
            first_multiple = prime_square;
        }

        for (long long multiple = first_multiple; multiple <= end; multiple += prime)
        {
            is_composite[multiple - start] = true;
        }
    }

    int prime_count = 0;
    for (size_t i = 0; i < window_size; ++i)
    {
        if (!is_composite[i])
        {
            ++prime_count;
        }
    }

    long long *primes = NULL;
    if (prime_count > 0)
    {
        primes = (long long *)malloc((size_t)prime_count * sizeof(long long));
        if (primes == NULL)
        {
            free(is_composite);
            fprintf(stderr, "Failed to allocate local primes buffer\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        int index = 0;
        for (size_t i = 0; i < window_size; ++i)
        {
            if (!is_composite[i])
            {
                primes[index++] = start + (long long)i;
            }
        }
    }

    free(is_composite);

    *primes_out = primes;
    *count_out = prime_count;
}
