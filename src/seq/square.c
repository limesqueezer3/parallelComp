#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

void square(float *x, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        x[i] = x[i] * x[i];
    }
}

void init(float *x, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        x[i] = 2.f;
    }
}

/* Summing many floats of equal size is horribly inaccurate, so accumulate
 * in double. */
double sum(float *x, size_t n)
{
    double res = 0.0;

    for (size_t i = 0; i < n; i++) {
        res += (double)x[i];
    }

    return res;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: %s N\n"
               "\tN: number of floats in your vector\n", argv[0]);
        return EXIT_FAILURE;
    }

    size_t n = atol(argv[1]);

    float *x = malloc(n * sizeof(float));

    init(x, n);

    struct timeval tv1, tv2;
    gettimeofday(&tv1, NULL);

    square(x, n);

    gettimeofday(&tv2, NULL);

    double duration = (double) (tv2.tv_usec - tv1.tv_usec) * 1e3 +
                      (double) (tv2.tv_sec - tv1.tv_sec) * 1e9;

    double check = sum(x, n);

    fprintf(stderr, "Compute rate %lf Gflops/s\n", (double)n / duration);
    fprintf(stderr, "Error is %.17e\n", fabs(4.0 * n - check));

    printf("%.17lf\n", (double)n / duration);

    free(x);

    return EXIT_SUCCESS;
}
