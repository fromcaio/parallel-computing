#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static long long parse_upper_bound(int argc, char **argv);

int main(int argc, char **argv)
{
    long long upper_bound = parse_upper_bound(argc, argv);
    if (upper_bound < 2)
    {
        return EXIT_FAILURE;
    }

    bool *is_prime = (bool *)malloc((size_t)(upper_bound + 1) * sizeof(bool));
    if (is_prime == NULL)
    {
        fprintf(stderr, "Unable to allocate sieve array for N=%lld\n", upper_bound);
        return EXIT_FAILURE;
    }

    for (long long i = 0; i <= upper_bound; ++i)
    {
        is_prime[i] = true;
    }
    is_prime[0] = false;
    if (upper_bound >= 1)
    {
        is_prime[1] = false;
    }

    for (long long candidate = 2; candidate <= upper_bound / candidate; ++candidate)
    {
        if (!is_prime[candidate])
        {
            continue;
        }

        long long start = candidate * candidate;
        for (long long multiple = start; multiple <= upper_bound; multiple += candidate)
        {
            is_prime[multiple] = false;
        }
    }

    int prime_count = 0;
    for (long long value = 2; value <= upper_bound; ++value)
    {
        if (is_prime[value])
        {
            printf("%lld\n", value);
            ++prime_count;
        }
    }

    printf("Total primes up to %lld: %d\n", upper_bound, prime_count);

    free(is_prime);
    return EXIT_SUCCESS;
}

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
        fprintf(stderr, "Invalid upper bound '%s'. Please provide an integer >= 2.\n", argv[1]);
        return -1;
    }

    return value;
}
