#include <stdio.h>
#include <omp.h>
#include <stdlib.h>

int main()
{

  printf("I think");

  #pragma omp parallel
  {
    #pragma omp single
    {
      #pragma omp task
      {
        /* annoying trick to get seeming randomness */
        for (int i = 0; i < rand() % 100; i++) {}
        printf(" car");
      }
      #pragma omp task
      printf(" race");
    }
  }

  printf("s are fun\n");

}
