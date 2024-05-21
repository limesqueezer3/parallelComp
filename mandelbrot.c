#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <sys/time.h>
#include <mpi.h>

/* Global variables */
int my_rank;
int p;

/* Returns largest divisor of p smaller than sqrt(p) */
int int_sqrt(int p)
{
    int q = 1;
    int best_yet = q;

    while (q * q <= p) {
        if (p % q == 0) {
            best_yet = q;
        }
        q++;
    }

    return best_yet;
}

/* Returns a divided by b, rounded up */
int ceildiv(int a, int b)
{
    return (a + b - 1) / b;
}

int min(int a, int b)
{
    return (a < b) ? a : b;
}

/* Computes the Mandelbrot set on [x1, x2] x [y1, y2]
 * at a m x n resolution. The value of picture[i, j] is
 * the number of iterations before bailout. */
void mandelbrot(int m, int n, double x1, double x2, double y1, double y2,
                int max_iter, int s1, int s2, int p1, int p2, int *picture /* out */)
{
    /* Let z = a + bi, c = d + ei */
    for (int i = s2; i < m; i += p2) {
        for (int j = s1; j < n; j += p1) {
            /* 8 flops */
            double complex c =  x1 + (x2 - x1) * i / m + 
                               (y1 + (y2 - y1) * j / n) * I;
            double complex z = 0;
            int t = 0;

            /* GCC squares the inequality, so computes z^2 <= 4
             * a^2 + b^2 <= 4, so 3 flops */
            while ((cabs(z) <= 2) && t < max_iter) {
                /* computes a^2 - b^2 + c + (2ab + d)i, so 7 flops */
                z = z * z + c;
                t++;
            }

            /* Total flops for this pixel are 14 + 7 * t. */
            picture[j * m + i] = t;
        }
    }
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    if (argc != 8) {
        printf("Usage: %s M N MAX_ITER x1 x2 y1 y2\n"
               "\tcreates an M x N pixel picture of [x1, x2] x [y1, y2]\n"
               "\tescaping after MAX_ITER iterations\n", argv[0]);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    int p1 = int_sqrt(p);
    int p2 = p / p1;

    int s1 = my_rank % p1;
    int s2 = my_rank / p1;

    int m        = atoi(argv[1]);
    int n        = atoi(argv[2]);
    int max_iter = atoi(argv[3]);

    double x1    = atof(argv[4]);
    double x2    = atof(argv[5]);
    double y1    = atof(argv[6]);
    double y2    = atof(argv[7]);

    int *picture = malloc(m * n * sizeof(int));
    int *gathered_picture = NULL;

    if (my_rank == 0) {
        gathered_picture = malloc(p * m * n * sizeof(int));
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

    mandelbrot(m, n, x1, x2, y1, y2, max_iter, s1, s2, p1, p2, picture);
    MPI_Barrier(MPI_COMM_WORLD);
    double stop = MPI_Wtime();
    MPI_Gather(picture, m * n, MPI_INT, gathered_picture, m * n, MPI_INT, 0, MPI_COMM_WORLD);
    
    double duration = (stop - start) * 1e9;
    
    if (my_rank == 0) {
        /* Write pgm to stderr. */
        fprintf(stderr, "P2\n");
        fprintf(stderr, "%d %d\n", m, n);
        fprintf(stderr, "%d\n", max_iter);
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                fprintf(stderr, "%d ", gathered_picture[i * n + j]);
            }
            fprintf(stderr, "\n");
        }

        /* Compute flops */
        double flops = 8 * m * n;
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                flops += 10 * gathered_picture[j * m + i];
            }
        }

        printf("%lf\n", flops / 1e9 / duration);
    }
    free(picture);
    
    MPI_Finalize();
    return EXIT_SUCCESS;
}