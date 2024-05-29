#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <mpi.h>

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

int main(int argc, char **argv)
{
    MPI_SAFE(MPI_Init(&argc, &argv));

    char host[80];

    if (gethostname(host, 80) != 0) {
        host[0] = '\0';
    }

    MPI_SAFE(MPI_Comm_rank(MPI_COMM_WORLD, &my_rank));
    MPI_SAFE(MPI_Comm_size(MPI_COMM_WORLD, &p));

    printf("Hello world from %s, rank %d/%d running on CPU %d!\n",
        host, my_rank, p, sched_getcpu());

    MPI_SAFE(MPI_Finalize());
}
