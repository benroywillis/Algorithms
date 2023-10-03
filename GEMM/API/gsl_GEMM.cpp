#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <time.h>
#include "TimingLib.h"

int main(int argc, char** argv)
{
  if( argc < 6 || argc > 7 )
  {
    printf("This application computes alpha*A*B + beta*C (BLAS L3). Please provide a, b, c matrix dimensions (axb*bxc), alpha and beta coefficients (in that order), and 0 or 1 (set matrix transpose to false or true)\n");
  }
  size_t a = atoi(argv[1]);
  size_t b = atoi(argv[2]);
  size_t c = atoi(argv[3]);
  float alpha = atoi(argv[4]);
  float beta = atoi(argv[5]);
  bool transpose = atoi(argv[6]) == 1 ? true : false;

  gsl_matrix_float *A = gsl_matrix_float_alloc (a, b);
  gsl_matrix_float *B = gsl_matrix_float_alloc (b, c);
  gsl_matrix_float *C = gsl_matrix_float_alloc (a, c);

  int ret;
  if( transpose )
  {
	__TIMINGLIB_benchmark( [&] { gsl_blas_sgemm(CblasTrans, CblasTrans, alpha, A, B, beta, C); } );
  }
  else
  {
	__TIMINGLIB_benchmark( [&] { gsl_blas_sgemm(CblasTrans, CblasTrans, alpha, A, B, beta, C); } );
  }
  printf("BLAS DGEMM successful\n");

  gsl_matrix_float_free(A);
  gsl_matrix_float_free(B);
  gsl_matrix_float_free(C);
  return 0;
}
