#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>
#include <time.h>
#include "fftw3.h"

int main(int argc, char** argv)
{
	struct timespec start, end;
	while( clock_gettime(CLOCK_MONOTONIC, &start) ) {}
	if( argc != 3)
	{
		printf("Please input the size of the desired DFT (N) and the dimension of this input\n");
	}
	// read in both the dimension degree and total DFTs to be formed
	int no_dfts = atoi(argv[1]);
	int degree = atoi(argv[2]);

	// create fftw_complex input
#if PRECISION == 0
	fftw_complex* in  = (fftw_complex* )fftw_malloc( (int)pow( (float)no_dfts, (float)degree) * sizeof(fftw_complex));
	fftw_complex* out = (fftw_complex* )fftw_malloc( (int)pow( (float)no_dfts, (float)degree) * sizeof(fftw_complex));
#else
	fftwf_complex* in  = (fftwf_complex* )fftwf_malloc((int)pow((float)no_dfts, (float)degree) * sizeof(fftwf_complex));
	fftwf_complex* out = (fftwf_complex* )fftwf_malloc((int)pow((float)no_dfts, (float)degree) * sizeof(fftwf_complex));
#endif
	for( unsigned int i = 0; i < (unsigned int)pow((double)no_dfts, (double)degree); i++ )
	{
		in[i] = (double)rand() + (double)rand()*I;
	}

	// create a plan and instantiate it
	int n[degree];
	for(int i = 0; i < degree; i++) 
    {
		n[i] = no_dfts;
	} // multidimensional DFTs will always be square

#if PRECISION == 0
	printf("Making double precision plan...\n");
	fftw_plan p = fftw_plan_dft(degree, n, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);
#else
	printf("Making single precision plan...\n");
	fftwf_plan p = fftwf_plan_dft(degree, n, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);
#endif

	// execute the plan
#if PRECISION == 0
	printf("Executing double precision plan...\n");
	fftw_execute(p);
#else
	printf("Executing single precision plan...\n");
	fftwf_execute(p);
#endif

#if PRECISION == 0
	fftw_free(in);
	fftw_free(out);
	fftw_destroy_plan(p);
#else
	fftwf_free(in);
	fftwf_free(out);
	fftwf_destroy_plan(p);
#endif
	while( clock_gettime(CLOCK_MONOTONIC, &end) ) {}
	double diff   = (double)end.tv_sec - (double)start.tv_sec;
	double diff_n = ((double)end.tv_nsec - (double)start.tv_nsec) * pow(10.0, -9.0);
	double total  = diff + diff_n;
	printf("Time: %f\n", total);
	return 0;
}
