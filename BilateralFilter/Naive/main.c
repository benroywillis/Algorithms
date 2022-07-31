#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "BilateralFilter.h"

int main(int argc, char** argv)
{
	if( argc !=5 )
	{
		printf("Please provide spacial variance sigma_s, range variance sigma_r, input image name, and output image name\n");
		return 1;
	}
	sigma_s = strtof(argv[1], NULL);
	sigma_r = strtof(argv[2], NULL);
	struct Pixel* input;
	PRECISION* output;
	input = readImage(argv[3]);
	output = (PRECISION*)calloc(image_width*image_height, sizeof(PRECISION));
	
	struct timespec start;
	struct timespec end;
	while(clock_gettime(CLOCK_MONOTONIC, &start));
	BilateralFilter(input, output);
	while(clock_gettime(CLOCK_MONOTONIC, &end));
	double secdiff = (double)(end.tv_sec - start.tv_sec);
	double nsecdiff = (double)(end.tv_nsec - start.tv_nsec) * pow(10.0, -9.0);
	double totalTime = secdiff + nsecdiff;
	printf("Bilateral filter runtime: %fs\n", totalTime);

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
	writeImage(input, argv[4]);

	free(input);
	free(output);
	return 0;
}
