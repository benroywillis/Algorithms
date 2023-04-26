
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define COLOR_DIM 	3
#define SIZE 		64
#define K			5
#define L			5
#define PRECISION 	float

const float kernel[K][L] = { {1.0f / 273.0f,  4.0f / 273.0f,  7.0f / 273.0f,  4.0f / 273.0f, 1.0f / 273.0f},
                       		 {4.0f / 273.0f, 16.0f / 273.0f, 26.0f / 273.0f, 16.0f / 273.0f, 4.0f / 273.0f},
                       		 {7.0f / 273.0f, 26.0f / 273.0f, 41.0f / 273.0f, 26.0f / 273.0f, 7.0f / 273.0f},
                        	 {4.0f / 273.0f, 16.0f / 273.0f, 26.0f / 273.0f, 16.0f / 273.0f, 4.0f / 273.0f},
                       		 {1.0f / 273.0f,  4.0f / 273.0f,  7.0f / 273.0f,  4.0f / 273.0f, 1.0f / 273.0f} };

struct Pixel {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

struct Pixel filter(PRECISION p[COLOR_DIM], int y, int x)
{
	static PRECISION newPixel[3];
	memset(newPixel, 0, K*L*sizeof(PRECISION));
	for( int k = -K/2; k < K/2; k++ )
	{
		for( int l = -L/2; l < L/2; l++ )
		{
			// boundary conditions are "reflective" ie cba | abcdef | fed
			int row = y+k < SIZE ? abs(y+k) : y-k;
			int col = x+l < SIZE ? abs(x+l) : x-l;
			for( int m = 0; m < COLOR_DIM; m++ )
			{
				newPixel[m] += p[m] * kernel[k+K/2][l+L/2];
			}
		}
	}
	struct Pixel acc;
	acc.r = newPixel[0];
	acc.g = newPixel[1];
	acc.b = newPixel[2];
	return acc;
}

struct Pixel filter_inPlace(PRECISION p[COLOR_DIM], int y, int x)
{
	for( int k = -K/2; k < K/2; k++ )
	{
		for( int l = -L/2; l < L/2; l++ )
		{
			int row = y+k < SIZE ? abs(y+k) : y-k;
			int col = x+l < SIZE ? abs(x+l) : x-l;
			for( int m = 0; m < COLOR_DIM; m++ )
			{
				p[m] += p[m] * kernel[k+K/2][l+L/2];
			}
		}
	}
	struct Pixel acc;
	acc.r = p[0];
	acc.g = p[1];
	acc.b = p[2];
	return acc;
}

void gauss_filter(struct Pixel* in, struct Pixel* out, bool inPlace)
{
	for( int y = 0; y < SIZE; y++ )
	{
		for( int x = 0; x < SIZE; x++ )
		{
			PRECISION pixel[COLOR_DIM] = { (PRECISION)in[y*SIZE + x].r, (PRECISION)in[y*SIZE + x].g, (PRECISION)in[y*SIZE + x].b };
			if( inPlace )
			{
				in[y*SIZE + x] = filter_inPlace( pixel, y, x );
			}
			else
			{
				out[y*SIZE + x] = filter( pixel, y, x );
			}
		}
	}	
}

int main(int argc, char** argv)
{
	// we run each version twice to verify two things:
	// - the epoch profile rejects the false dependence between the out-of-place version
	// - accepts the dependence between the in-place version

	// first the out-of-place version
	struct Pixel* img      = (struct Pixel*)malloc(SIZE*SIZE*sizeof(struct Pixel));	
	for( unsigned i = 0; i < SIZE*SIZE; i++ )
	{
		img[i].r = (uint8_t)rand();
		img[i].g = (uint8_t)rand();
		img[i].b = (uint8_t)rand();
	}
	struct Pixel* filtered = (struct Pixel*)malloc(SIZE*SIZE*sizeof(struct Pixel));	
	memset(filtered, 0, SIZE*SIZE*sizeof(struct Pixel));
	gauss_filter(img, filtered, false);
	gauss_filter(img, filtered, false);
	// second, in-place version
	// we do this back-to-back to verify the epoch profile can detect their dependence, while rejecting their 
	gauss_filter(img, filtered, true);
	gauss_filter(filtered, filtered, true);
	return 0;
}
