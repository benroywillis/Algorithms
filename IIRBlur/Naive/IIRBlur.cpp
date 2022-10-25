
#include <stdlib.h>
#include <stdio.h>
#include "BilateralFilter.h"
#include "TimingLib.h"

//#define IMAGE_HEIGHT 240
//#define IMAGE_WIDTH  239

void IIR_Blur_cols(struct Pixel* input, struct Pixel* output, float alpha, unsigned int num_rows, unsigned int num_cols)
{
	// set the first row of the output to the input
	for( unsigned x = 0; x < num_cols; x++ )
	{
		output[x] = input[x];
	}
	// blur down the columns
	for( unsigned int x = 0; x < num_cols; x++ )
	{
		for( unsigned int y = 1; y < num_rows; y++ )
		{
			output[y*num_cols + x].r = (1-alpha)*input[(y-1)*num_cols + x].r + alpha*input[y*num_cols + x].r;
			output[y*num_cols + x].g = (1-alpha)*input[(y-1)*num_cols + x].g + alpha*input[y*num_cols + x].g;
			output[y*num_cols + x].b = (1-alpha)*input[(y-1)*num_cols + x].b + alpha*input[y*num_cols + x].b;
		}
	}
	// blur up the columns
	for( unsigned int x = 0; x < num_cols; x++ )
	{
		for( int y = num_rows - 2; y >= 0; y-- )
		{
			//output[y*num_cols + x].r = (1-alpha)*input[(y+1)*num_cols + x].r + alpha*input[y*num_cols + x].r;
			//output[y*num_cols + x].g = (1-alpha)*input[(y+1)*num_cols + x].g + alpha*input[y*num_cols + x].g;
			//output[y*num_cols + x].b = (1-alpha)*input[(y+1)*num_cols + x].b + alpha*input[y*num_cols + x].b;
			output[y*num_cols + x].r = (1-alpha)*output[(y+1)*num_cols + x].r + alpha*output[y*num_cols + x].r;
			output[y*num_cols + x].g = (1-alpha)*output[(y+1)*num_cols + x].g + alpha*output[y*num_cols + x].g;
			output[y*num_cols + x].b = (1-alpha)*output[(y+1)*num_cols + x].b + alpha*output[y*num_cols + x].b;
		}
	}
}

void IIR_Blur_rows(struct Pixel* input, struct Pixel* output, float alpha, unsigned int num_rows, unsigned int num_cols)
{
	// set the first col of the output to the input
	for( unsigned y = 0; y < num_rows; y++ )
	{
		output[y*num_cols] = input[y*num_cols];
	}
	// blur along the rows left to right
	for( unsigned int y = 0; y < num_rows; y++ )
	{
		for( unsigned int x = 1; x < num_cols; x++ )
		{
			output[y*num_cols + x].r = (1-alpha)*input[y*num_cols + (x-1)].r + alpha*input[y*num_cols + x].r;
			output[y*num_cols + x].g = (1-alpha)*input[y*num_cols + (x-1)].g + alpha*input[y*num_cols + x].g;
			output[y*num_cols + x].b = (1-alpha)*input[y*num_cols + (x-1)].b + alpha*input[y*num_cols + x].b;
		}
	}
	// blur along the rows right to left
	for( unsigned int y = 0; y < num_rows; y++ )
	{
		for( int x = num_cols-2; x >= 0; x-- )
		{
			//output[y*num_cols + x].r = (1-alpha)*input[y*num_cols + (x+1)].r + alpha*input[y*num_cols + x].r;
			//output[y*num_cols + x].g = (1-alpha)*input[y*num_cols + (x+1)].g + alpha*input[y*num_cols + x].g;
			//output[y*num_cols + x].b = (1-alpha)*input[y*num_cols + (x+1)].b + alpha*input[y*num_cols + x].b;
			output[y*num_cols + x].r = (1-alpha)*output[y*num_cols + (x+1)].r + alpha*output[y*num_cols + x].r;
			output[y*num_cols + x].g = (1-alpha)*output[y*num_cols + (x+1)].g + alpha*output[y*num_cols + x].g;
			output[y*num_cols + x].b = (1-alpha)*output[y*num_cols + (x+1)].b + alpha*output[y*num_cols + x].b;
		}
	}
}

void filter(struct Pixel* input, struct Pixel* output, float alpha, unsigned int num_rows, unsigned int num_cols)
{
	Pixel buffer[num_rows][num_cols];
	// blur cols
	IIR_Blur_cols(input, (Pixel*)&buffer, alpha, num_rows, num_cols);
	// blur rows
	IIR_Blur_rows((Pixel*)&buffer, output, alpha, num_rows, num_cols);
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
	//image_width  = IMAGE_WIDTH;
	//image_height = IMAGE_HEIGHT;
	struct Pixel* output = (struct Pixel*)calloc(image_width*image_height, sizeof(struct Pixel));
	printf("Image size: %d x %d\n", image_height, image_width);

	__TIMINGLIB_benchmark( [&]() { filter(input, output, alpha, image_height, image_width); } );

	writeImage(output, argv[3]);

	free(input);
	free(output);

	printf("Done\n");

	return 0;
}
