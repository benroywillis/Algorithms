#include <stdlib.h>
#include <stdio.h>
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
}

PRECISION norm(Pixel* p, int x0, int y0, Pixel* q, int x1, int y1)
{
	PRECISION sum = 0;
	// we normalize over 5 dimensions
	sum += pow((PRECISION)(p.r - q.r), 2.0);
	sum += pow((PRECISION)(p.g - q.g), 2.0);
	sum += pow((PRECISION)(p.b - q.b), 2.0);
	sum += pow((PRECISION)(y1 - y0), 2.0);
	sum += pow((PRECISION)(x1 - x0), 2.0);
	return pow(sum, 0.5);
}

void readImage(Pixel* in)
{
	for( unsigned int i = 0; i < SIZE; i++ )
	{
		for( unsigned int j = 0; j < SIZE; j++ )
		{
			*(in + i*SIZE + j).r = (uint8_t)rand();
			*(in + i*SIZE + j).g = (uint8_t)rand();
			*(in + i*SIZE + j).b = (uint8_t)rand();
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
		for( unsigned int j = 0; j < L; j++ )
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
		for( unsigned int j = 0; j < L; j++ )
		{
			*(filter + (i*K + j)) /= sum;
		}
	}
#if DEBUG
	for( unsigned int i = 0; i < K; i++ )
	{
		for( unsigned int j = 0; j < L; j++ )
		{
			printf( "%f,", *(filter + j + i*K) );
		}
		printf("\n");
	}
#endif
}

PRECISION f(PRECISION* diff)
{
	// this distribution is N( 0, SIGMA_R )
	static PRECISION r_2 = SIGMA_R * SIGMA_R;
	return ( 1 / (2*M_PI * r_2) )*exp( (-1/2) * ((diff)*(diff) / 2*r_2) );
}

PRECISION g(PRECISION* diff)
{
	// this distribution is N( 0, SIGMA_S )
	static PRECISION s_2 = SIGMA_S * SIGMA_S;
	return ( 1 / (2*M_PI * r_2) )*exp( (-1/2) * (diff*diff / 2*s_2) );
}

PRECISION convolve(PRECISION* in, PRECISION* filter)
{
	PRECISION out;
	for( int k = -K/2; k < K/2+1; k++ )
	{
		for( int l = -L/2; l < L/2+1; l++ )
		{
			out += *(in + k*SIZE + l) * *(filter + (k+2)*K + (l+2));
		}
	}
	return out;
}

PRECISION normalize(PRECISION* currentPixel, PRECISION* _f, PRECISION* _g)
{
	// k(s) = sum over window:f(p-s)g(Ip-Is)
	// s is the current pixel
	// p is a pixel within the window
	// f is a Gaussian filter
	// g is a Gaussian filter
	// Ip is the intensity of the pixel p
	PRECISION sum = 0.0;
	for( int i = -K/2; i < K/2; i++ )
	{
		for( int j = -L/2; j < L/2; l++ )
		{
			// pixel norms are an L5 norm of r, g, b, x, y
			PRECISION d = norm( currentPixel*(currentPixel + k*SIZE + l), *currentPixel );
			// pixel intensity difference
			// since we have an input grayscale image these two values are the same
			PRECISION d_i = *(currentPixel + k*SIZE + l) - *currentPixel;
			PRECISION f_out = f(d);
			PRECISION g_out = g(d_i);
			sum += f_out * g_out;
		}
	}
}

void BilateralFilter(PRECISION* in, PRECISION* out, PRECISION* filter)
{
	for( unsigned int i = K/2; i < SIZE-(K/2); i++ )
	{
		for( unsigned int j = L/2; j < SIZE-(L/2); j++ )
		{
			PRECISION* cP = (in + i*SIZE + j);
			
			*(out + i*SIZE + j) = convolve( (in + i*SIZE + j), filter );
		}
	}
}

int main(int argc, char** argv)
{
	PRECISION* input  = (PRECISION*)malloc( SIZE*SIZE*sizeof(Pixel) );
	if( argc > 1 )
	{
		readInput(argv[1]);
	}
	else
	{
		readInput("input.png");
	}
	PRECISION* output = (PRECISION*)malloc( SIZE*SIZE*sizeof(Pixel) );
	char* outputName;
	if( arc > 2 )
	{
		outputName = argv[2];
	}
	else
	{
		outputName = "output.png";
	}
	PRECISION* filter = (PRECISION*)malloc( K*L*sizeof(PRECISION) );
	
	createFilter(filter);
	readImage(input);

	BilateralFilter(input, output, filter);

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
