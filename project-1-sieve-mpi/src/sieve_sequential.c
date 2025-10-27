/**
 * @file sieve_sequential.c
 * @brief Sequential implementation of the Sieve of Eratosthenes.
 *
 * This program computes all prime numbers up to a given integer N using
 * the classic sieve algorithm. It writes the resulting primes to a file
 * named "primes-sequential.txt" and prints execution statistics to stdout.
 *
 * Usage:
 *     ./bin/sieve_seq 1000000
 *
 * Author: Caio Reis
 */

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>  // For execution time measurement

/**
 * @brief Parses the command-line argument and returns the upper bound N.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return The parsed upper bound, or -1 if invalid.
 */
static long long parse_upper_bound(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s N\n", argv[0]);
        return -1;
    }

    errno = 0;
    char *endptr = NULL;
    long long value = strtoll(argv[1], &endptr, 10);

    if (errno != 0 || endptr == argv[1] || *endptr != '\0' || value < 2)
    {
        fprintf(stderr, "Invalid input '%s'. Please provide an integer >= 2.\n", argv[1]);
        return -1;
    }

    return value;
}

/**
 * @brief Entry point: executes the sequential sieve.
 */
int main(int argc, char **argv)
{
    long long upper_bound = parse_upper_bound(argc, argv);
    if (upper_bound < 2)
    {
        return EXIT_FAILURE;
    }

    // Start timing only the computation phase
    clock_t start_time = clock();

    // Allocate a boolean array representing [0, upper_bound]
    bool *is_prime = malloc((size_t)(upper_bound + 1) * sizeof(bool));
    if (is_prime == NULL)
    {
        fprintf(stderr, "Error: Unable to allocate memory for N = %lld\n", upper_bound);
        return EXIT_FAILURE;
    }

    // Initialize all entries as true; 0 and 1 are not primes.
    for (long long i = 0; i <= upper_bound; ++i)
        is_prime[i] = true;

    is_prime[0] = false;
    if (upper_bound >= 1)
        is_prime[1] = false;

    // Core sieve algorithm: mark multiples as non-prime.
    for (long long candidate = 2; candidate * candidate <= upper_bound; ++candidate)
    {
        if (!is_prime[candidate])
            continue;

        // Start from candidate² to avoid redundant markings
        for (long long multiple = candidate * candidate; multiple <= upper_bound; multiple += candidate)
            is_prime[multiple] = false;
    }

    // Stop timing before writing to disk (I/O excluded from measurement)
    clock_t end_time = clock();
    double elapsed = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    // Write results to file
    FILE *output = fopen("primes-sequential.txt", "w");
    if (output == NULL)
    {
        fprintf(stderr, "Error: Unable to create output file.\n");
        free(is_prime);
        return EXIT_FAILURE;
    }

    int prime_count = 0;
    for (long long value = 2; value <= upper_bound; ++value)
    {
        if (is_prime[value])
        {
            fprintf(output, "%lld\n", value);
            ++prime_count;
        }
    }
    fclose(output);

    // Print summary to console
    printf("[Sequential] Computed %d primes up to N = %lld\n", prime_count, upper_bound);
    printf("[Sequential] Execution time (computation only): %f seconds\n", elapsed);
    printf("[Sequential] Output written to primes-sequential.txt\n");

    free(is_prime);
    return EXIT_SUCCESS;
}