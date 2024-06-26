#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <sys/time.h>
#include <mpi.h>

void mandelbrot(int m, int n, double x1, double x2, double y1, double y2,
                int max_iter, int *picture /* out */, int rank, int size, int *Nthreshold, int *load)
{
    //Each process computes a portion (divided horizontally by columns)
    int i_start = rank * m / size;
    int i_end = (rank + 1) * m / size;

    for (int i = i_start; i < i_end; i++) {
        for (int j = 0; j < n; j++) {
            double complex c =  x1 + (x2 - x1) * i / m + 
                               (y1 + (y2 - y1) * j / n) * I;
            double complex z = 0;
            int t = 0;

            while ((cabs(z) <= 2) && t < max_iter) {
                z = z * z + c;
                t++;
            }
            /* Total flops for this pixel are 14 + 7 * t. */
            if (t == max_iter) {
                (*Nthreshold)++;
            }
            *load += t;
            picture[(i - i_start) * n + j] = t;
        }
    }
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 8) {
        printf("Usage: %s M N MAX_ITER x1 x2 y1 y2\n"
                "\tcreates an M x N pixel picture of [x1, x2] x [y1, y2]\n"
                "\tescaping after MAX_ITER iterations\n", argv[0]);
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    //Store variables in all processes
    int m;
    int n;
    int max_iter;
    double x1;
    double x2;
    double y1;
    double y2;
    int NThresholdLocal = 0;       // Number of values at the iteration threshold
    int *NThresholdGlobal = NULL;  // Total number of values at the iteration threshold
    int loadLocal = 0;            // flops of local process
    int *loadGlobalMax = NULL;    // most flops done by any process
    int *loadGlobalMin = NULL;    // least flops done by any process

    //But only root process needs to read parameters
    if(rank == 0) {
        m        = atoi(argv[1]);
        n        = atoi(argv[2]);
        max_iter = atoi(argv[3]);
        x1       = atof(argv[4]);
        x2       = atof(argv[5]);
        y1       = atof(argv[6]);
        y2       = atof(argv[7]);
    }


    int *picture = malloc(m * n * sizeof(int));

    /* Distribute blocks to processes (assume 4 for now), compute them */
    // Broadcast the parameters to all processes
    MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&max_iter, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&x1, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&x2, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&y1, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&y2, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    int *full_picture = NULL;
    int *displs = NULL;
    int *recvcounts = NULL;
    if (rank == 0) {
        full_picture = malloc(m * n * sizeof(int));
        displs = malloc(size * sizeof(int));
        recvcounts = malloc(size * sizeof(int));
        for (int i = 0; i < size; i++) {
            displs[i] = i * m / size * n;
            recvcounts[i] = (i + 1) * m / size * n - displs[i];
        }
        NThresholdGlobal = malloc(sizeof(int));
        loadGlobalMax = malloc(sizeof(int));
        loadGlobalMin = malloc(sizeof(int));
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

    // Call the mandelbrot function on each process
    // printf("rank %d, received param: %i %i %i %f %f %f %f\n", rank, m, n ,max_iter, x1, x2, y1, y2);
    mandelbrot(m, n, x1, x2, y1, y2, max_iter, picture, rank, size, &NThresholdLocal, &loadLocal);

    MPI_Barrier(MPI_COMM_WORLD);
    double stop = MPI_Wtime();
    double duration = stop - start;

    int i_start = rank * m / size;
    int i_end = (rank + 1) * m / size;

    //Gatherv instead of gather because number of points may differ by processes
    MPI_Gatherv(picture, (i_end - i_start) * n, MPI_INT, full_picture, recvcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Reduce(&NThresholdLocal, NThresholdGlobal, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&loadLocal, loadGlobalMax, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&loadLocal, loadGlobalMin, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        // Write pgm to stderr. 
        fprintf(stderr, "P2\n");
        fprintf(stderr, "%d %d\n", m, n);
        fprintf(stderr, "%d\n", max_iter);
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                fprintf(stderr, "%d ", full_picture[j * m + i]);
            }
            fprintf(stderr, "\n");
        }

        /* Compute flops */
        double flops = 8 * m * n;
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                flops += 10 * picture[j * m + i];
            }
        }

        free(gathered_picture);
        // printf("Flops : %f \n", flops);                             // Total flops based on all pixels,

        /* compute percentage of pixels at threshold*/
        printf("%lf\n", duration);
        printf("%lf\n", flops / 1e9 / duration);                   // GLOPS/s
        // fprintf(stderr, "%lf\n", percentage);

        /* print loadbalance*/
        // double loadbalance = (double) *loadGlobalMax / *loadGlobalMin;
        // fprintf(stderr, "loadBalance is %lf\n", loadbalance); 
    }
    free(picture);

    MPI_Finalize();

    return EXIT_SUCCESS;
}
