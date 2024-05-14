
#include <stdlib.h>
#include <stdio.h>
#include "TimingLib.h"
#include "BilateralFilter.h"

#ifndef PRECISION
#define TYPE float
#elif PRECISION == 0
#define TYPE float
#elif PRECISION == 1
#define TYPE double
#else
#define TYPE double
#endif

#define K 5
#define L 5

// discrete gaussian kernel approximation with mu = 0 sigma = 1
const TYPE filter[K][L] = { {1.0f / 273.0f,  4.0f / 273.0f,  7.0f / 273.0f,  4.0f / 273.0f, 1.0f / 273.0f},
                            {4.0f / 273.0f, 16.0f / 273.0f, 26.0f / 273.0f, 16.0f / 273.0f, 4.0f / 273.0f},
                            {7.0f / 273.0f, 26.0f / 273.0f, 41.0f / 273.0f, 26.0f / 273.0f, 7.0f / 273.0f},
                            {4.0f / 273.0f, 16.0f / 273.0f, 26.0f / 273.0f, 16.0f / 273.0f, 4.0f / 273.0f},
                            {1.0f / 273.0f,  4.0f / 273.0f,  7.0f / 273.0f,  4.0f / 273.0f, 1.0f / 273.0f} };

void ImageConv( TYPE* image, TYPE* blur ) {
	for( int y = 0; y < image_height; y++ ) {
		for( int x = 0; x < image_width; x++ ) {
			for( int k = -K/2; k < K/2; k++ ) {
				// boundary conditions "reflect" off the boundaries
				int row = y+k < 0 ? abs(y+k) : y+k;
				for( int l = -L/2; l < L/2; l++ ) {
					// boundary conditions "reflect" off the boundaries
					int col = x+l < 0 ? abs(x+l) : x+l;
					row     = y+k >= image_height ? y-k : y+k;
					col     = x+l >= image_width ? x-l : x+l;
					blur[y*image_width] += filter[k][l] * image[row*image_width+col];
				}
			}
		}
	}
}

int main(int argc, char** argv) {
    if( argc != 3 )
    {
        printf("Please provide input image path, and output image path\n");
        return 1;
    }
    struct Pixel* input;
    TYPE* gray;
    TYPE* output;
    input  = readImage(argv[1]);
	gray   = (TYPE*)calloc(image_width*image_height, sizeof(TYPE));
    output = (TYPE*)calloc(image_width*image_height, sizeof(TYPE));

    __TIMINGLIB_benchmark( [&]{ 
		// gray the image
		for( unsigned y = 0; y < image_height; y++ ) {
			for( unsigned x = 0; x < image_width; x++ ) {
				gray[y*image_width+x] = ((TYPE)0.299)*input[y*image_width+x].r + ((TYPE)0.587)*input[y*image_width+x].g + ((TYPE)0.114)*input[y*image_width+x].b;
			}
		}
		ImageConv(gray, output);
	});

    // convert output image to an image acceptable for printing
    // use the input memory space
    for( unsigned int i = 0; i < image_height; i++ )
    {
        for( unsigned int j = 0; j < image_width; j++ )
        {
            (input + i*image_height + j)->r = (uint8_t)(*(output + i*image_height + j));
            (input + i*image_height + j)->g = (input + i*image_height + j)->r;
            (input + i*image_height + j)->b = (input + i*image_height + j)->g;
        }
    }

    // the output space is grayscale
    writeImage(input, argv[2]);

    free(input);
    free(output);
    return 0;
}
