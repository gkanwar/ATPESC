#include <stdio.h>
#include <omp.h>
#include <stdlib.h>

int main()
{

  #pragma omp parallel
  {
    #pragma omp single
    {
      for (int i = 0; i < 10; i++) {
        #pragma omp task
        printf("0");
        #pragma omp task
        printf("1");
      }
    }
  }
  printf("\n");
  
}
