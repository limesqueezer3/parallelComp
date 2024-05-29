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

void PrintArray(int m, int n, int s, int p)
{
    printf("Hello from P(%d)! My indices are:\n", s);

    int b = ceildiv(n, p);

    for (int j = s * b; j < min((s + 1) * b, n); j++) {
        for (int i = 0; i < m; i++) {
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


    if (s == 0) {
        printf("1D block distribution over the first index:\n\n");
    }
    MPI_Barrier(MPI_COMM_WORLD);

    /* The loops and barriers are to make sure the arrays are printed 
     * sequentially. Do not do this in the assignment! */
    for (int t = 0; t < p; t++) {
        if (t == s) {
            PrintArray(m, n, s, p);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    MPI_Finalize();
}
