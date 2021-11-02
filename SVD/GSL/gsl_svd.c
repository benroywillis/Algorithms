#include <gsl/gsl_linalg.h>

int main(int argc, char** argv)
{
	if( argc != 3 )
	{
		printf("Please provide M and N as argument\n");
	}
	int m = atoi(argv[1]);
	int n = atoi(argv[2]);

	gsl_matrix* A = gsl_matrix_alloc((size_t)m, (size_t)n);
	gsl_matrix* V = gsl_matrix_alloc((size_t)n, (size_t)n);
	gsl_vector* S = gsl_vector_alloc((size_t)n);
	gsl_vector* work = gsl_vector_alloc((size_t)n);

	int ret = gsl_linalg_SV_decomp(A, V, S, work);
	if( ret )
	{
		printf("SVD call returned error code %d\n", ret);
		return EXIT_FAILURE;
	}
	return 0;
}
