#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <time.h>

int main(int argc, char** argv)
{
  struct timespec start, end;
  while( clock_gettime(CLOCK_MONOTONIC, &start) ) {}
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

  gsl_matrix *A = gsl_matrix_alloc (a, b);
  gsl_matrix *B = gsl_matrix_alloc (b, c);
  gsl_matrix *C = gsl_matrix_alloc (a, c);

  int ret;
  if( transpose )
  {
    ret = gsl_blas_dgemm(CblasTrans, CblasTrans, alpha, A, B, beta, C);
  }
  else
  {
    ret = gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, alpha, A, B, beta, C);
  }
  if( ret )
  {
    printf("BLAS DGEMM call returned error code %d\n", ret);
  }
  printf("BLAS DGEMM successful\n");

  gsl_matrix_free(A);
  gsl_matrix_free(B);
  gsl_matrix_free(C);
  while( clock_gettime(CLOCK_MONOTONIC, &end) ) {}
  double diff = (double)end.tv_sec - (double)start.tv_sec;
  double ndiff = ((double)end.tv_nsec - (double)start.tv_nsec) * pow(10.0, -9.0);
  double total = diff + ndiff;
  printf("Time: %fs\n", total);
  return 0;
}
