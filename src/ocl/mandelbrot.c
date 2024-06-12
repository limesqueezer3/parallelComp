/* See ocl-simple for a more verbose version. This prints only the time in
 * seconds to allow for easy benchmarking. */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <CL/cl.h>
#include "simple.h"

#define DATA_SIZE 10240000

int main (int argc, char * argv[])
{
  cl_kernel kernel;
  size_t global[2];
  size_t local[1];
  /* Must be run from top directory, otherwise this path is wrong! */
  char *KernelSource = readOpenCL( "src/ocl/mandelbrot.cl");

  if (argc != 9) {
      printf("Usage: %s <workgroup size> M N MAX_ITER x1 x2 y1 y2\n"
               "\tcreates an M x N pixel picture of [x1, x2] x [y1, y2]\n"
               "\tescaping after MAX_ITER iterations\n", argv[0]);
      return EXIT_FAILURE;
  }

  local[0] = atoi(argv[1]);
    int m        = atoi(argv[2]);
    int n        = atoi(argv[3]);
    int max_iter = atoi(argv[4]);

    double x1    = atof(argv[5]);
    double x2    = atof(argv[6]);
    double y1    = atof(argv[7]);
    double y2    = atof(argv[8]);
    int count = m*n;
    int *pictureReturned = malloc(count * sizeof(int));
    int mn_max_iter[3] = {m, n, max_iter};
    double xy[4] = {x1, x2, y1, y2};

  fprintf(stderr, "work group size: %d\n", (int)local[0]);

  /* Create data for the run.  */
  int correct;                       /* Number of correct results returned.  */


  global[0] = (m + local[0] -1) / local[0] * local[0];
  global[1] = (n + local[0] -1) / local[0] * local[0];


  CL_SAFE(initGPU());
  struct timeval tv1, tv2;
  gettimeofday(&tv1, NULL);

  kernel = setupKernel( KernelSource, "mandelbrot", 3, IntArr, count, pictureReturned,
                                                      DoubleArr, 4, xy,
                                                      IntArr, 3, mn_max_iter);

  runKernel( kernel, 2, global, local);

  gettimeofday(&tv2, NULL);

  double duration = (double) (tv2.tv_usec - tv1.tv_usec) / 1e6 +
                    (double) (tv2.tv_sec - tv1.tv_sec) / 1e9;

  /* Compute flops */
    double flops = 8 * m * n;
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            flops += 10 * pictureReturned[j * m + i];
        }
    }
  printf("%lf\n", flops / duration / 1e9);




  CL_SAFE(clReleaseKernel (kernel));
  CL_SAFE(freeDevice());
  free(pictureReturned);

  return 0;
}
