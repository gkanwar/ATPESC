/*

  This program will numerically compute the integral of

  4/(1+x*x) 
				  
  from 0 to 1.  The value of this integral is pi -- which 
  is great since it gives us an easy way to check the answer.

  The is the original sequential program.  It uses the timer
  from the OpenMP runtime library

  History: Written by Tim Mattson, 11/99.

*/
#include <stdio.h>
#include <omp.h>
/* static long num_steps = 1000000000; */
static int total_depth = 32;
double step;

double integrate(double start, double end, int depth) {
  double x = 0.5*(start+end);
  if (depth == 0) {
    return (end-start)*4.0/(1.0+x*x);
  }
  else if (depth < 25) {
    double s1, s2;
    s1 = integrate(start, x, depth-1);
    s2 = integrate(x, end, depth-1);
    return s1+s2;
  }
  else {
    double s1, s2;
    #pragma omp task shared(s1)
    s1 = integrate(start, x, depth-1);
    #pragma omp task shared(s2)
    s2 = integrate(x, end, depth-1);
    #pragma omp taskwait
    return s1+s2;
  }
}

int main ()
{
  int i;
  double pi, sum = 0.0;
  double start_time, run_time;

  /* step = 1.0/(double) num_steps; */

        	 
  start_time = omp_get_wtime();

  
  /* for (i=1;i<= num_steps; i++){ */
  /*   double x = (i-0.5)*step; */
  /*   sum = sum + 4.0/(1.0+x*x); */
  /* } */

  #pragma omp parallel
  {
    #pragma omp single
    pi = integrate(0.0, 1.0, total_depth);
  }
  run_time = omp_get_wtime() - start_time;
  printf("\n pi with %d depth is %lf in %lf seconds\n ", total_depth, pi, run_time);
}	  





