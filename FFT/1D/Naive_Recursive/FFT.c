#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>
#include <time.h>

#define PRECISION double
#ifndef SIZE
#define SIZE	512
#endif

void _fft(PRECISION complex buf[], PRECISION complex out[], int n, int step)
{
	if (step < n) {
		_fft(out, buf, n, step * 2);
		_fft(out + step, buf + step, n, step * 2);
		int i;
		for (i = 0; i < n; i += 2 * step) {
			PRECISION complex t1 = (PRECISION) cexp(-I * M_PI * i / n);
		    PRECISION complex t = t1 * out[i + step];
			buf[i / 2]     = out[i] + t;
			buf[(i + n)/2] = out[i] - t;
		}
	}
}

int main(int argc, char** argv)
{
	if( argc != 2)
	{
		printf("Please input the size of the desired DFT (N)");
	}
	struct timespec start, end;
    int size = atoi(argv[1]);
	size = SIZE;
    // Instantiate In/Out arrays
    PRECISION complex in_rec[size], in_loop[SIZE], out_rec[size], out_loop[SIZE];
	// Generate some random data
    for( unsigned int i = 0; i < size; i++ )
	{
		in_rec[i] = (PRECISION) rand() + (PRECISION) rand()*I;
		in_loop[i] = in_rec[i];
        out_rec[i] = in_rec[i];
        out_loop[i] = in_rec[i];
	}

 	while( clock_gettime(CLOCK_MONOTONIC, &start) ) {}
    // Do the FFT
    _fft(in_rec,out_rec,size,1);
	printf("Recursive FFT done\n");
	while( clock_gettime(CLOCK_MONOTONIC, &end) ) {}

	double diff   = (double)end.tv_sec - (double)start.tv_sec;
	double diff_n = ((double)end.tv_nsec - (double)start.tv_nsec) * pow(10.0, -9.0);
	double totalTime  = diff + diff_n;
	printf("Time: %f\n", totalTime);
	return 0;
}
