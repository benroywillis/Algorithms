#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define PRECISION 	double
#define SIZE 		512
#define K			5
#define SIGMA		1.0

void readImage(PRECISION* in)
{
	for( unsigned int i = 0; i < SIZE; i++ )
	{
		for( unsigned int j = 0; j < SIZE; j++ )
		{
			*(in + i*SIZE + j) = rand();
		}
	}
}

void createFilter(PRECISION* filter)
{
	double r = 2.0 * SIGMA * SIGMA;
	double s = r;
	double sum = 0.0;

	for( unsigned int i = 0; i < K; i++ )
	{
		for( unsigned int j = 0; j < K; j++ )
		{
			int l = i - 2;
			int m = j - 2;
			r = sqrt(l*l + m*m);
			*(filter + (i*K + j)) = exp( -(r*r) / s ) / (M_PI * s);
			sum += *(filter + (i*K + j));
		}
	}
	for( unsigned int i = 0; i < K; i++ )
	{
		for( unsigned int j = 0; j < K; j++ )
		{
			*(filter + (i*K + j)) /= sum;
		}
	}

/*
	for( unsigned int i = 0; i < K; i++ )
	{
		for( unsigned int j = 0; j < K; j++ )
		{
			printf( "%f,", *(filter + j + i*K) );
		}
		printf("\n");
	}
*/

}

PRECISION convolve(PRECISION* in, PRECISION* filter)
{
	PRECISION out;
	for( int k = -K/2; k < K/2+1; k++ )
	{
		for( int l = -K/2; l < K/2+1; l++ )
		{
			out += *(in + k*SIZE + l) * *(filter + (k+2)*SIZE + l+2);
		}
	}
	return out;
}

void blur(PRECISION* in, PRECISION* out, PRECISION* filter)
{
	for( unsigned int i = K/2; i < SIZE-(K/2); i++ )
	{
		for( unsigned int j = K/2; j < SIZE-(K/2); j++ )
		{
			*(out + i*SIZE + j) = convolve( (in + i*SIZE + j), filter );
		}
	}
}

int main()
{
	PRECISION* input  = (PRECISION*)malloc( SIZE*SIZE*sizeof(PRECISION) );
	PRECISION* output = (PRECISION*)malloc( SIZE*SIZE*sizeof(PRECISION) );
	PRECISION* filter = (PRECISION*)malloc( K*K*sizeof(PRECISION) );
	
	createFilter(filter);
	readImage(input);

	blur(input, output, filter);

/*	for( unsigned int i = 0; i < SIZE; i++ )
	{
		for( unsigned int j = 0; j < SIZE; j++ )
		{
			printf( "%f,", *(output+ j + i*SIZE) );
		}
		printf("\n");
	}*/

	return 0;
}
