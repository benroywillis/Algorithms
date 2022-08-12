
#include <stdlib.h>
#include <stdio.h>
#include "BilateralFilter.h"
#include "TimingLib.h"

#define IMAGE_HEIGHT 240
#define IMAGE_WIDTH  239

void transpose(struct Pixel* image, unsigned int num_rows, unsigned int num_cols)
{
	for( unsigned int i = 0; i < num_rows; i++ )
	{
		for( unsigned int j = 0; j < num_cols; j++ )
		{
			(image + i*num_cols + j)->r = (image + (num_rows-i-1)*num_cols + (num_cols-j-1))->r;
			(image + i*num_cols + j)->g = (image + (num_rows-i-1)*num_cols + (num_cols-j-1))->g;
			(image + i*num_cols + j)->b = (image + (num_rows-i-1)*num_cols + (num_cols-j-1))->b;
		}
	}
}

void IIR_Blur(struct Pixel* input, struct Pixel* output, float alpha, unsigned int num_rows, unsigned int num_cols)
{
	// set the first row of the output to the input
	for( unsigned i = 0; i < num_cols; i++ )
	{
		*(output + i) = *(input + i);
	}
	// blur down the columns
	for( unsigned int i = 0; i < num_cols; i++ )
	{
		for( unsigned int j = 0; j < num_rows; j++ )
		{
			(output + j*num_cols + i)->r = (1-alpha) * (input + j*num_cols + i - 1)->r + (alpha) * (input + j*num_cols + i)->r;
			(output + j*num_cols + i)->g = (1-alpha) * (input + j*num_cols + i - 1)->g + (alpha) * (input + j*num_cols + i)->g;
			(output + j*num_cols + i)->b = (1-alpha) * (input + j*num_cols + i - 1)->b + (alpha) * (input + j*num_cols + i)->b;
		}
	}
	// blur up the columns
	for( unsigned int i = num_cols - 2; i > 0; i-- )
	{
		for( unsigned int j = 0; j < num_rows; j++ )
		{
			(output + j*num_cols + i)->r = (1-alpha) * (input + j*num_cols + i + 1)->r + (alpha) * (input + j*num_cols + i)->r;
			(output + j*num_cols + i)->g = (1-alpha) * (input + j*num_cols + i + 1)->g + (alpha) * (input + j*num_cols + i)->g;
			(output + j*num_cols + i)->b = (1-alpha) * (input + j*num_cols + i + 1)->b + (alpha) * (input + j*num_cols + i)->b;
		}
	}
	
}

void filter(struct Pixel* input, struct Pixel* output, float alpha, unsigned int num_rows, unsigned int num_cols)
{
	// blur cols
	IIR_Blur(input, output, alpha, num_rows, num_cols);
	// transpose
	transpose(output, num_rows, num_cols);
	// blur rows
	IIR_Blur(input, output, alpha, num_cols, num_rows);
	// transpose back to normal
	transpose(output, num_cols, num_rows);
}

int main(int argc, char** argv)
{
	if( argc != 4 )
	{
		printf("Please provide (float)alpha, input image name and output image name.\n");
		return EXIT_FAILURE;
	}
	float alpha = strtof(argv[1], NULL);
	struct Pixel* input  = readImage(argv[2]);
	image_width  = IMAGE_WIDTH;
	image_height = IMAGE_HEIGHT;
	struct Pixel* output = (struct Pixel*)calloc(image_width*image_height, sizeof(struct Pixel));
	printf("Image size: %d x %d\n", image_height, image_width);

	__TIMINGLIB_benchmark( [&]() { filter(input, output, alpha, image_height, image_width); } );

	writeImage(input, argv[3]);

	free(input);
	free(output);

	printf("Done\n");

	return 0;

}
