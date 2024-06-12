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
                int max_iter, int s1, int s2, int p1, int p2, int *picture, int *Nthreshold, int *load /* out */)
{
    int counter = 0;
    /* Let z = a + bi, c = d + ei */
    for (int i = s1; i < m; i += p1) {
        for (int j = s2; j < n; j += p2) {
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
            if (t == max_iter) {
                (*Nthreshold)++;
            }
            *load += t;
            picture[counter] = t;
            counter++;
        }
    }
    // printf("Nthreshold %d\n", *Nthreshold);
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
    if(my_rank == 0) {
        m        = atoi(argv[1]);
        n        = atoi(argv[2]);
        max_iter = atoi(argv[3]);
        x1       = atof(argv[4]);
        x2       = atof(argv[5]);
        y1       = atof(argv[6]);
        y2       = atof(argv[7]);
    }
    
    // Broadcast the parameters to all processes
    MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&max_iter, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&x1, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&x2, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&y1, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&y2, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    int max_amount = (m /p1 + 1) * (n/p2 + 1);
    int *picture = malloc(max_amount * sizeof(int));
    int *gathered_picture = NULL;

    if (my_rank == 0) {
        gathered_picture = malloc(p* max_amount * sizeof(int));
        NThresholdGlobal = malloc(sizeof(int));
        loadGlobalMax = malloc(sizeof(int));
        loadGlobalMin = malloc(sizeof(int));
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

    mandelbrot(m, n, x1, x2, y1, y2, max_iter, s1, s2, p1, p2, picture, &NThresholdLocal, &loadLocal);
    MPI_Barrier(MPI_COMM_WORLD);
    double stop = MPI_Wtime();
    double duration = stop - start;


    int *displs = malloc(p*sizeof(int));
    int *rcounts = malloc(p*sizeof(int));
    for(int i = 0; i <p; i++) {
        displs[i] = i * max_amount;
        rcounts[i] = max_amount;
    }
    MPI_Gatherv(picture, max_amount, MPI_INT, gathered_picture, rcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Reduce(&NThresholdLocal, NThresholdGlobal, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&loadLocal, loadGlobalMax, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&loadLocal, loadGlobalMin, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
   
    
    if (my_rank == 0) {
        /* compute flops based on gathered picture instead of true picture. 
        Also possible on true picture if picture is reconstructed below*/
        double flops = 8 * m * n;
        for (int q = 0; q < p; q++) {
            int counter = max_amount * q;
            int s1 = q % p1;
            int s2 = q / p1;
            for (int i = s1; i < m ; i+= p1) {
                for (int j = s2; j < n ; j+= p2) {
                    flops += 10 * gathered_picture[counter];
                    counter++;
                }
            }
        }

        /* reconstruct the picture*/
        // int *true_picture = malloc(m * n * sizeof(int));
        // for (int q = 0; q < p; q++) {
        //     int counter = max_amount * q;
        //     int s1 = q % p1;
        //     int s2 = q / p1;
        //     for (int i = s1; i < m ; i+= p1) {
        //         for (int j = s2; j < n ; j+= p2) {
        //             true_picture[j * m + i] = gathered_picture[counter];
        //             counter++;
        //         }
        //     }
        // }
        
        /* Write pgm to stderr. */
        // fprintf(stderr, "P2\n");
        // fprintf(stderr, "%d %d\n", m, n);
        // fprintf(stderr, "%d\n", max_iter);
        // for (int i = 0; i < m; i++) {
        //     for (int j = 0; j < n; j++) {
        //         fprintf(stderr, "%d ", true_picture[i * n + j]);
        //     }
        //     fprintf(stderr, "\n");
        // }
        // free(true_picture);

        free(gathered_picture);
        /* compute percentage of pixels at threshold*/
        double percentage = (double) *NThresholdGlobal / (m * n);
        printf("%lf\n", flops);
        printf("%lf\n", flops / 1e9 / duration);
        fprintf(stderr, "%lf\n", percentage);

        /* print loadbalance*/
        // double loadbalance = (double) *loadGlobalMax / *loadGlobalMin;
        // fprintf(stderr, "loadBalance is %lf\n", loadbalance);
    }
    free(picture);
    
    MPI_Finalize();
    return EXIT_SUCCESS;
}