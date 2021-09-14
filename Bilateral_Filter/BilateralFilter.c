#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#define PRECISION 	double
#define SIZE 		512
#define K			5
#define L			5
#define SIGMA_S		1.0
#define SIGMA_R		1.0
#define R			2 * SIGMA_R * SIGMA_R
#define S 			2 * SIGMA_S * SIGMA_S

struct Pixel
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

void readImage(struct Pixel* in, char* file)
{
	for( unsigned int i = 0; i < SIZE; i++ )
	{
		for( unsigned int j = 0; j < SIZE; j++ )
		{
			(in + i*SIZE + j)->r = (uint8_t)rand();
			(in + i*SIZE + j)->g = (uint8_t)rand();
			(in + i*SIZE + j)->b = (uint8_t)rand();
		}
	}
}

PRECISION norm_space(struct Pixel* p, int x0, int y0, struct Pixel* q, int x1, int y1)
{
	PRECISION sum = 0;
	// we normalize over 5 dimensions
	sum += pow((PRECISION)(p->r - q->r), 2.0);
	sum += pow((PRECISION)(p->g - q->g), 2.0);
	sum += pow((PRECISION)(p->b - q->b), 2.0);
	sum += pow((PRECISION)(y1 - y0), 2.0);
	sum += pow((PRECISION)(x1 - x0), 2.0);
	return pow(sum, 0.5);
}

// converts rgb to grayscale
PRECISION Intensity(struct Pixel* a)
{
	return (PRECISION)a->r / 3.0 + (PRECISION)a->g / 3.0 + (PRECISION)a->b / 3.0; 
}

// computes the spacial component
PRECISION f(PRECISION diff)
{
	// this distribution is N( 0, SIGMA_S )
	static PRECISION s_2 = SIGMA_S * SIGMA_S;
	return ( 1 / (2*M_PI * s_2) )*exp( (-1/2) * (diff*diff / 2*s_2) );
}

// computes the range component
PRECISION g(PRECISION diff)
{
	// this distribution is N( 0, SIGMA_R )
	static PRECISION r_2 = SIGMA_R * SIGMA_R;
	return ( 1 / (2*M_PI * r_2) )*exp( (-1/2) * ((diff)*(diff) / 2*r_2) );
}

void BilateralFilter(struct Pixel* in, PRECISION* out)
{
	// for each pixel
	for( unsigned int i = K/2; i < SIZE-(K/2); i++ )
	{
		for( unsigned int j = L/2; j < SIZE-(L/2); j++ )
		{
			struct Pixel* cP = (in + i*SIZE + j);
			// BF[p] = 1 / Wp * SUM( f(|| p-q ||)g(| Ip-Iq |)Ip )
			// Wp = SUM( f(|| p-q ||)g(| Ip-Iq |) )
			// p is the current pixel
			// q is a pixel within the window
			// f is a Gaussian filter for space
			// g is a Gaussian filter for range
			// Ip is the intensity of the pixel p
			PRECISION Wp = 0.0;
			for( int k = -K/2; k < K/2; k++ )
			{
				for( int l = -L/2; l < L/2; l++ )
				{
					// pixel norms are an L5 norm of r, g, b, x, y
					PRECISION d = norm_space( cP, i, j, cP + k*SIZE + l, i+k, j+l );
					// pixel intensity difference
					// since we have an input grayscale image these two values are the same
					PRECISION d_i = fabs( Intensity(cP + k*SIZE + l) - Intensity(cP) );
					PRECISION f_out = f(d);
					PRECISION g_out = g(d_i);
					PRECISION mul = f_out * g_out;
					*(out + i*SIZE + j) = mul*Intensity(cP + k*SIZE + l);
					Wp += mul;
				}
			}
			*(out + i*SIZE + j) /= Wp;
		}
	}
}

int main(int argc, char** argv)
{
	// the input space is rgb with 3-word width
	struct Pixel* input  = (struct Pixel*)malloc( SIZE*SIZE*sizeof(struct Pixel) );
	if( argc > 1 )
	{
		readImage(input, argv[1]);
	}
	else
	{
		readImage(input, "input.png");
	}
	// the output space is grayscale
	PRECISION* output = (PRECISION*)malloc( SIZE*SIZE*sizeof(PRECISION) );
	char* outputName;
	if( argc > 2 )
	{
		outputName = argv[2];
	}
	else
	{
		outputName = "output.png";
	}

	struct timespec start;
	struct timespec end;
	while(clock_gettime(CLOCK_MONOTONIC, &start));
	BilateralFilter(input, output);
	while(clock_gettime(CLOCK_MONOTONIC, &end));
	double secdiff = (double)(end.tv_sec - start.tv_sec);
	double nsecdiff = (double)(end.tv_nsec - start.tv_nsec) * pow(10.0, -9.0);
	double totalTime = secdiff + nsecdiff;
	printf("Bilateral filter runtime: %fs\n", totalTime);

#ifdef DEBUG
	for( unsigned int i = 0; i < SIZE; i++ )
	{
		for( unsigned int j = 0; j < SIZE; j++ )
		{
			printf( "%f,", *(output+ j + i*SIZE) );
		}
		printf("\n");
	}
#endif
	return 0;
}
