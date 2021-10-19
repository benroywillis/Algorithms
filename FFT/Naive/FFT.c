#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>
#include <time.h>

void _fft(float complex buf[], float complex out[], int n, int step)
{
	if (step < n) {
		_fft(out, buf, n, step * 2);
		_fft(out + step, buf + step, n, step * 2);
		int i;
		for (i = 0; i < n; i += 2 * step) {
			float complex t1 = cexp(-I * M_PI * i / n);
		    float complex t = t1 * out[i + step];
			buf[i / 2]     = out[i] + t;
			buf[(i + n)/2] = out[i] - t;
		}
	}
}

int main(int argc, char** argv)
{
	struct timespec start, end;
	while( clock_gettime(CLOCK_MONOTONIC, &start) ) {}
	if( argc != 2)
	{
		printf("Please input the size of the desired DFT (N)");
	}

    int size = atoi(argv[1]);
    // Instantiate In/Out arrays
    float complex in[size], out[size];
	// Generate some random data
    for( unsigned int i = 0; i < size; i++ )
	{
		in[i] = (double)rand() + (double)rand()*I;
        out[i]= in[i];
	}

    // Do the FFT
    _fft(in,out,size,1);


	while( clock_gettime(CLOCK_MONOTONIC, &end) ) {}
	double diff   = (double)end.tv_sec - (double)start.tv_sec;
	double diff_n = ((double)end.tv_nsec - (double)start.tv_nsec) * pow(10.0, -9.0);
	double total  = diff + diff_n;
	printf("Time: %f\n", total);
	return 0;
}
