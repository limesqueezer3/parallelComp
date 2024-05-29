#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define MPI_SAFE(fncall) do {                                                 \
   int retcode = (fncall);                                                    \
   if (retcode != MPI_SUCCESS) {                                              \
       char msg[1024];                                                        \
       int resultlen;                                                         \
       MPI_Error_string(retcode, msg, &resultlen);                            \
       fprintf(stderr, "Call %s failed with %s(%d)\n",                        \
                    #fncall, msg, retcode);                                   \
       MPI_Abort(MPI_COMM_WORLD, retcode);                                    \
   }                                                                          \
} while (0)

/* Global variables */
int my_rank;
int p;

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

size_t roundUp(size_t a, size_t b)
{
    return (a + b - 1) / b;
}

/* Summing many floats of equal size is horribly inaccurate, so accumulate
 * in double. Must be called collectively. */
double sum(float *x, size_t n)
{
    double res = 0.0;
    double my_res = 0.0;

    for (size_t i = 0; i < n; i++) {
        my_res += (double)x[i];
    }

    double all_res[p];    
    MPI_SAFE(MPI_Allgather(&my_res, 1, MPI_DOUBLE,
                           all_res, 1, MPI_DOUBLE,
                           MPI_COMM_WORLD));

    for (int r = 0; r < p; r++) {
        res += all_res[r];
    }

    return res;
}

int main(int argc, char **argv)
{
    MPI_SAFE(MPI_Init(&argc, &argv));

    if (argc != 2) {
        printf("Usage: %s N\n"
               "\tN: number of floats in your vector\n", argv[0]);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    size_t n = atol(argv[1]);
    MPI_SAFE(MPI_Comm_rank(MPI_COMM_WORLD, &my_rank));
    MPI_SAFE(MPI_Comm_size(MPI_COMM_WORLD, &p));

    /* Block distribution, all processors except the last one have this many
     * elements. This minimizes the maximum of elements per processor. */
    size_t blocksize = roundUp(n, p);
    if (my_rank == p - 1) {
        blocksize = n - (p - 1) * blocksize;
    }

    float *x = malloc(blocksize * sizeof(float));

    init(x, blocksize);

    /* Barrier is necessary to get accurate timings. */
    MPI_SAFE(MPI_Barrier(MPI_COMM_WORLD));
    double start = MPI_Wtime();

    square(x, blocksize);

    MPI_SAFE(MPI_Barrier(MPI_COMM_WORLD));
    double stop = MPI_Wtime();

    double duration = (stop - start) * 1e9;

    double check = sum(x, blocksize);

    fprintf(stderr, "Rank %d measures compute rate %lf Gflops/s\n", 
            my_rank, (double)n / duration);
    fprintf(stderr, "Rank %d says error is %.17e\n", 
            my_rank, fabs(4.0 * n - check));

    if (my_rank == 0) {
        printf("%.17lf\n", (double)n / duration);
    }

    free(x);

    MPI_SAFE(MPI_Finalize());
}
