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
    double z = 0;
    \\ implement further

    
    pictureReturned[j * m + i] = ;
   } else {
    pictureReturned[j * m + i] = 0;
   }
   
}
