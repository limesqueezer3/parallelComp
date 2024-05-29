#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

/* Returns a divided by b, rounded up */
int ceildiv(int a, int b)
{
    return (a + b - 1) / b;
}

int min(int a, int b)
{
    return (a < b) ? a : b;
}

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

void PrintArray(int m, int n, int s1, int s2, int p1, int p2)
{
    printf("Hello from P(%d, %d)! My indices are:\n", s1, s2);

    int b1 = ceildiv(m, p1);
    int b2 = ceildiv(n, p2);

    for (int j = s2 * b2; j < min((s2 + 1) * b2, n); j++) {
        for (int i = s1 * b1; i < min((s1 + 1) * b1, m); i++) {
            printf("(%.2d, %.2d)", i, j);
        }
        printf("\n");
    }
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    if (argc != 3) {
        printf("Usage: mpirun -n <nr of ranks> %s <m> <n>\n"
               "\tWill distribute an m x n array\n", argv[0]);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int s, p;

    MPI_Comm_rank(MPI_COMM_WORLD, &s);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    int m = atoi(argv[1]);
    int n = atoi(argv[2]);

    int p1 = int_sqrt(p);
    int p2 = p / p1;

    int s1 = s % p1;
    int s2 = s / p1;

    if (s == 0) {
        printf("2D cyclic distribution over a [%d, %d] processor grid:\n\n",
                p1, p2);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    /* The loops and barriers are to make sure the arrays are printed 
     * sequentially. Do not do this in the assignment! */
    for (int t1 = 0; t1 < p1; t1++) {
        for (int t2 = 0; t2 < p2; t2++) {
            if (t1 == s1 && t2 == s2) {
                PrintArray(m, n, s1, s2, p1, p2);
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
}
