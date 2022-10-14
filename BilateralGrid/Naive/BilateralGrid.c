
// Inspiration taken from https://jonhwayim.medium.com/seminar-from-bilateral-filtering-to-hdr-applications-b644428119d4

#include <stdlib.h>
#include <stdio.h>
#include "BilateralFilter.h"

#define MAX_INTENSITY 255

#define SAMPLE_RATE_S 16
#define SAMPLE_RATE_R 16

unsigned int grid_height = 0;
unsigned int grid_width  = 0;
unsigned int grid_depth  = 0;

PRECISION* buildGrid(struct Pixel* image)
{
	grid_height = image_height % SAMPLE_RATE_S  ? image_height  / SAMPLE_RATE_S + 1 : image_height  / SAMPLE_RATE_S;
	grid_width  = image_width  % SAMPLE_RATE_S  ? image_width   / SAMPLE_RATE_S + 1 : image_width   / SAMPLE_RATE_S;
	grid_depth  = MAX_INTENSITY % SAMPLE_RATE_R ? MAX_INTENSITY / SAMPLE_RATE_R + 1 : MAX_INTENSITY / SAMPLE_RATE_R;
	// the grid has size rows/samplerate_s x columns/samplerate_s x max(INTENSITY)/samplerate_r
	// the grid has two entries for each index
	// 1. an accumulated intensity value
	// 2. a discrete count of members in that index
	PRECISION* grid = (PRECISION*)calloc(grid_height*grid_width*grid_depth*2, sizeof(PRECISION));
	printf("Grid has base pointer 0x%lx and holds %d elements.\n", (uint64_t)grid, grid_height*grid_width*grid_depth*2);
	printf("Number of downsampled rows: %d\n", grid_height);
	printf("Number of downsampled cols: %d\n", grid_width);
	printf("Number of downsampled Intensities: %d\n", grid_depth);

	for( unsigned int i = 0; i < image_height; i++ )
	{
		for( unsigned int j = 0; j < image_width; j++ )
		{
			// downsampling occurs by dividing image coordinate by sample rate
			unsigned int row = (i/SAMPLE_RATE_S)*(grid_width)*(grid_depth)*2;
			unsigned int col = (j/SAMPLE_RATE_S)*(grid_depth)*2;
			unsigned int Int = ((unsigned int)Intensity((image + i*image_height + j))/SAMPLE_RATE_R)*2;
			*(grid + row + col + Int)     += Intensity((image + i*image_width+ j));
			*(grid + row + col + Int + 1) = (PRECISION)((unsigned int)*(grid + row + col + Int + 1) + 1);
		}
	}
	unsigned int totalAccumulatedPixels = 0;
	for( unsigned int i = 0; i < image_height/SAMPLE_RATE_S; i++ )
	{
		for( unsigned int j = 0; j < image_width/SAMPLE_RATE_S; j++ )
		{
			for( unsigned int k = 0; k < MAX_INTENSITY/SAMPLE_RATE_R; k++ )
			{
				PRECISION* entry = (grid + (i*grid_width*grid_depth*2)+(j*grid_depth*2)+k*2);
				printf("Entry %d, %d, %d (index %d) has accumulated intensity %g and membership count %g\n", i, j, k, i*grid_width*grid_depth*2+j*grid_depth*2+k*2, *(entry), *(entry+1));
				totalAccumulatedPixels += (unsigned int)*(entry+1);
			}
		}
	}
	if( totalAccumulatedPixels != image_height*image_width )
	{
		printf("ERROR! Total accumulated pixels was %d and total pixels in the image was %d!\n", totalAccumulatedPixels, image_height*image_width);
	}
	return grid;
}

void GaussianKernel(PRECISION* grid, float sigma_s, float sigma_r, float sigma_c)
{
	
}

int main(int argc, char** argv)
{
	if( argc != 5 )
	{
		printf("Please provide spacial and range variance parameters, input image name and output image name.\n");
		return EXIT_FAILURE;
	}
	sigma_s = strtof(argv[1], NULL);
	sigma_r = strtof(argv[2], NULL);
	struct Pixel* input = readImage(argv[3]);
	PRECISION* output   = (PRECISION*)calloc(image_width*image_height, sizeof(PRECISION));
	printf("Image size: %d x %d\n", image_height, image_width);

	// 1. Build bilateral grid
	PRECISION* grid = buildGrid(input);

	// 2. Convolve grid with gaussian filter

	// 3. "Slice" the grid into a 2D image by using trilinear interpolation

	// convert resulting image to grayscale
	for( unsigned int i = 0; i < image_height; i++ )
	{
		for( unsigned int j = 0; j < image_width; j++ )
		{
			(input + image_height*i + j)->r = (uint8_t)(*(output + i*image_height + j));
			(input + image_height*i + j)->g = (input + image_height*i + j)->r;
			(input + image_height*i + j)->b = (input + image_height*i + j)->r;
		}
	}

	writeImage(input, argv[4]);

	free(input);
	free(grid);
	free(output);
	return 0;
}
