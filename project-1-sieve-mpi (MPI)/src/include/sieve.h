#ifndef SIEVE_H
#define SIEVE_H

#include <stdbool.h>
#include <stdlib.h>

long long parse_upper_bound(int argc, char **argv, int rank);
void append_values(long long **array, int *capacity, int *count, const long long *values, int values_count);
int compare_long_long(const void *lhs, const void *rhs);
void write_primes_to_file(const char *filename, const long long *primes, int count);

long long integer_sqrt(long long value);
long long *generate_base_primes(long long n, int *count_out);
void compute_segment_bounds(long long n, int world_size, int rank, long long *start, long long *end);
void sieve_segment(long long start, long long end, const long long *base_primes, int base_count,
                   long long **primes_out, int *count_out);

#endif
