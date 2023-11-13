#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>
#include <time.h>

#define PRECISION double
#ifndef SIZE
#define SIZE	4096
#endif

// kept here for comparison.. It doesn't do the normalization step so the answers differ widely from the nonrecursive fft
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

// taken from https://www.codeproject.com/Articles/619688/Quick-FFT, is under the code project open license
void BitInvert( PRECISION complex buf[], int n )
{
	int k, rev, mv;
	PRECISION complex a;
	for( int i = 1; i < n; i++ )
	{
		k   = i;
		mv  = n/2;
		rev = 0;
		while( k > 0 )
		{
			if( (k%2) > 0 )
			{
				rev += mv;
			}
			k  /= 2;
			mv /= 2;
		}
		{
			if( i < rev )
			{
				a = buf[rev];
				buf[rev] = buf[i];
				buf[i] = a;
			}
		}
	}
}

void calcSubFFT( PRECISION complex buf[], int n )
{
	int k, m;
	PRECISION complex w, v, h;
	k = 1;
	while( k <= n/2 )
	{
		m = 0;
		while( m <= (n-2*k) )
		{
			for( int i = m; i < m + k; i++ )
			{
				w = (PRECISION)cexp(-I * M_PI * i/n);
				h = buf[i+k] * w;
				v = buf[i];
				buf[i] = buf[i] + h;
				buf[i + k] = v - h;
			}
			m += 2*k;
		}
		k *= 2;
	}
}

void _fft_nonrecursive( PRECISION complex buf[] )
{
	BitInvert(buf, SIZE);
	calcSubFFT(buf, SIZE);
	for( int i = 0; i < SIZE; i++ )
	{
		buf[i] /= ( (PRECISION)SIZE * (PRECISION)2.0 );
	}
	buf[0] /= 2.0;
}

int main(int argc, char** argv)
{
	/*if( argc != 2)
	{
		printf("Please input the size of the desired DFT (N)");
	}
    int size = atoi(argv[1]);
	size = SIZE;*/
	struct timespec start, end;
    // Instantiate In/Out arrays
    PRECISION complex in_rec[SIZE], in_loop[SIZE], out_rec[SIZE], out_loop[SIZE];
	// Generate some random data
    for( unsigned int i = 0; i < SIZE; i++ )
	{
		in_rec[i] = (PRECISION) rand() + (PRECISION) rand()*I;
		in_loop[i] = in_rec[i];
        out_rec[i] = in_rec[i];
        out_loop[i] = in_rec[i];
	}

 	while( clock_gettime(CLOCK_MONOTONIC, &start) ) {}
    // Do the FFT
    //_fft(in_rec,out_rec,size,1);
	printf("Recursive FFT done\n");
    _fft_nonrecursive(in_loop);
	while( clock_gettime(CLOCK_MONOTONIC, &end) ) {}

	double diff   = (double)end.tv_sec - (double)start.tv_sec;
	double diff_n = ((double)end.tv_nsec - (double)start.tv_nsec) * pow(10.0, -9.0);
	double totalTime  = diff + diff_n;
	printf("Time: %f\n", totalTime);

	// compare answers
	/*double error = 0.0;
	double total = 0.0;
	for( unsigned i = 0; i < SIZE; i++ )
	{
		error += in_loop[i] - in_rec[i];
		total += in_loop[i] + in_rec[i];
	}
	printf("Total error was %g%%\n", (error/total) * 100);*/
	return 0;
}
