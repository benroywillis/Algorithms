#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "BilateralFilter.h"
#include "TimingLib.h"

#ifndef PRECISION
#define PRECISION float
#endif

#define K 	5
#define L 	5

void move(PRECISION* in, PRECISION* out, unsigned int height, unsigned int width)
{
#define IN(y, x) in[y*width + x]
#define OUT(y, x) out[y*width + x]
	for( int y = 0; y < height; y++ )
	{
		for( int x = 0; x < width; x++ )
		{
			OUT(y, x) = IN(y, x);
		}
	}
	memset(out, 0, height*width*sizeof(PRECISION));
}

void filter(PRECISION* in, PRECISION* out, unsigned int height, unsigned int width)
{
#define IN(y, x) in[y*width + x]
#define OUT(y, x) out[y*width + x]
	// discrete approximation to a gaussian smoothing filter with mean 0.0 and sigma = 1.0
	const PRECISION filter[K][L] = { 
									 {1.0f / 273.0f,  4.0f / 273.0f,  7.0f / 273.0f,  4.0f / 273.0f, 1.0f / 273.0f}, 
									 {4.0f / 273.0f, 16.0f / 273.0f, 26.0f / 273.0f, 16.0f / 273.0f, 4.0f / 273.0f}, 
									 {7.0f / 273.0f, 26.0f / 273.0f, 41.0f / 273.0f, 26.0f / 273.0f, 7.0f / 273.0f}, 
									 {4.0f / 273.0f, 16.0f / 273.0f, 26.0f / 273.0f, 16.0f / 273.0f, 4.0f / 273.0f}, 
									 {1.0f / 273.0f,  4.0f / 273.0f,  7.0f / 273.0f,  4.0f / 273.0f, 1.0f / 273.0f} 
								   };
//#pragma scop
	for( int y = 0; y < height; y++ )
	{
		for( int x = 0; x < width; x++ )
		{
			// the target pixel is in the middle of the filter, therefore we go from -K/2,-L/2 to K/2,L/2
			// for boundary conditions, we reflect the image -> cba | abcdef | fed
			for( int k = -K/2; k <= (K/2); k++ )
			{
				for( int l = -L/2; l <= (L/2); l++ )
				{
					//int row = abs(y + k);
					//int col = abs(x + l);
					int row = y + k < 0 ? 0 : y + k;
					int col = x + l < 0 ? 0 : x + l;
					OUT(y, x) += IN(row, col) * filter[k + K/2][l + L/2];
				}
			}
		}
	}
//#pragma endscop
}

int main(int argc, char** argv)
{
    if( argc != 3 )
    {
        printf("Please provide input image name and output image name.\n");
        return EXIT_FAILURE;
    }
    struct Pixel* input  = readImage(argv[1]);
	//image_height = 426;
	//image_width  = 640;
	//image_height = 1280;
	//image_width  = 1920;
    struct Pixel* output = (struct Pixel*)calloc(image_width*image_height, sizeof(struct Pixel));
    printf("Image size: %d x %d\n", image_height, image_width);

	PRECISION* in  = (PRECISION*)calloc(image_width*image_height,sizeof(PRECISION));
	PRECISION* out = (PRECISION*)calloc(image_width*image_height,sizeof(PRECISION));
    __TIMINGLIB_benchmark( [&]() { 
		// convert to floating point grayscale
		for( unsigned y = 0; y < image_height; y++ )
		{
			for( unsigned x = 0; x < image_width; x++ )
			{
				in[y*image_width + x] += (PRECISION)(0.299)*input[y*image_width + x].r;
				in[y*image_width + x] += (PRECISION)(0.587)*input[y*image_width + x].g;
				in[y*image_width + x] += (PRECISION)(0.114)*input[y*image_width + x].b;
			}
		}
		// stencil chain
		// between each pipe stage we put the result into the input array
		filter(in, out, image_height, image_width);
		move(out, in, image_height, image_width);
		filter(in, out, image_height, image_width);
		move(out, in, image_height, image_width);
		filter(in, out, image_height, image_width);
		move(out, in, image_height, image_width);
		filter(in, out, image_height, image_width);
		move(out, in, image_height, image_width);
		filter(in, out, image_height, image_width);
	} );

	// write output image
	for( unsigned y = 0; y < image_height; y++ )
	{
		for( unsigned x = 0; x < image_width; x++ )
		{
			output[y*image_width + x].r = (uint8_t)out[y*image_width + x];
			output[y*image_width + x].g = (uint8_t)out[y*image_width + x];
			output[y*image_width + x].b = (uint8_t)out[y*image_width + x];
		}
	}
    writeImage(output, argv[2]);

    free(input);
    free(output);
	free(in);
	free(out);
    return 0;
}
