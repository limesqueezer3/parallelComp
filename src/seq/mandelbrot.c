#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <sys/time.h>

/* Computes the Mandelbrot set on [x1, x2] x [y1, y2]
 * at a m x n resolution. The value of picture[i, j] is
 * the number of iterations before bailout. */
void mandelbrot(int m, int n, double x1, double x2, double y1, double y2,
                int max_iter, int *picture, int *Nthreshold, int *load /* out */)
{
    /* Let z = a + bi, c = d + ei */
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
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
            if (t == max_iter) {
                (*Nthreshold)++;
            }
            *load += t;
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 8) {
        printf("Usage: %s M N MAX_ITER x1 x2 y1 y2\n"
               "\tcreates an M x N pixel picture of [x1, x2] x [y1, y2]\n"
               "\tescaping after MAX_ITER iterations\n", argv[0]);
        return EXIT_FAILURE;
    }

    int m        = atoi(argv[1]);
    int n        = atoi(argv[2]);
    int max_iter = atoi(argv[3]);

    double x1    = atof(argv[4]);
    double x2    = atof(argv[5]);
    double y1    = atof(argv[6]);
    double y2    = atof(argv[7]);
    int load = 0;
    int Nthreshold = 0;

    int *picture = malloc(m * n * sizeof(int));

    struct timeval tv1, tv2;
    gettimeofday(&tv1, NULL);
    mandelbrot(m, n, x1, x2, y1, y2, max_iter, picture, &Nthreshold, &load);
    gettimeofday(&tv2, NULL);
    double duration = (double) (tv2.tv_usec - tv1.tv_usec) / 1e6 +
                      (double) (tv2.tv_sec - tv1.tv_sec);

    /* Write pgm to stderr. */
    // fprintf(stderr, "P2\n");
    // fprintf(stderr, "%d %d\n", m, n);
    // fprintf(stderr, "%d\n", max_iter);
    // for (int i = 0; i < m; i++) {
    //     for (int j = 0; j < n; j++) {
    //         fprintf(stderr, "%d ", picture[i * n + j]);
    //     }
    //     fprintf(stderr, "\n");
    // }

    /* Compute flops */
    double flops = 8 * m * n;
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            flops += 10 * picture[j * m + i];
        }
    }
    double percentage = (double) Nthreshold / (m * n) * 100;
    fprintf(stderr, "%lf\n", percentage);
    fprintf(stderr, "total load is %d\n", load);

    printf("number of flops %lf \n", flops);
    printf("%lf\n", flops / 1e9 / duration);
    printf("%lf duration \n", duration);

    free(picture);

    return EXIT_SUCCESS;
}
