#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <mpi.h>
#include <omp.h>

#define MPI_SAFE(fncall)                                                      \
    {                                                                         \
        if (fncall != MPI_SUCCESS) {                                          \
            perror("MPI call unsuccessfull\n");                               \
            MPI_Abort(MPI_COMM_WORLD, 1);                                     \
        }                                                                     \
    }

/* Global variables */
int my_rank;
int p;

int main(int argc, char **argv)
{
    int provided;
    MPI_SAFE(MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided));

    if (provided != MPI_THREAD_MULTIPLE) {
        printf("Cannot call MPI from multiple threads\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    char host[80];

    if (gethostname(host, 80) != 0) {
        host[0] = '\0';
    }

    MPI_SAFE(MPI_Comm_rank(MPI_COMM_WORLD, &my_rank));
    MPI_SAFE(MPI_Comm_size(MPI_COMM_WORLD, &p));

    #pragma omp parallel
    {
        printf("Hello world from %s, rank %d/%d, thread %d/%d running on CPU %d!\n",
            host, my_rank, p, omp_get_thread_num(), 
            omp_get_num_threads(), sched_getcpu());
    }

    MPI_SAFE(MPI_Finalize());
}
