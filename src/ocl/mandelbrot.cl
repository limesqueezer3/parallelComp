__kernel void mandelbrot(
   __global int* pictureReturned,
   __global double* xy,
   __global int* mn_max_iter)
{
   int i = get_global_id(0);
   int j = get_global_id(1);
   int m = mn_max_iter[0];
   int n = mn_max_iter[1];
   int max_iter = mn_max_iter[2];
   double x1 = xy[0];
   double x2 = xy[1];
   double y1 = xy[2];
   double y2 = xy[3];

   int iter = 0; 

   if (i < m && j < n) {
    double cComplex = y1 + (y2 - y1) * j / n;
    double cReal = x1 + (x2 - x1) * i / m;
    double zComplex = 0;
    double zReal = 0;
    double zSquaredComplex, zSquaredReal;
    
    while ((sqrt((zComplex * zComplex + zReal * zReal)) < 2)
            && iter < max_iter) {
               zSquaredReal = zReal * zReal - zComplex * zComplex;
               zSquaredComplex = zReal * zComplex + zReal * zComplex;
               zComplex = zSquaredComplex + cComplex;
               zReal = zSquaredReal + cReal; 
               iter++;
            } 
    pictureReturned[j * m + i] = iter;
   } else {
    pictureReturned[j * m + i] = 0;
   }
}
