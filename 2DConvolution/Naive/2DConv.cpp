
#include <stdlib.h>
#include <stdio.h>
#include "TimingLib.h"
#include "BilateralFilter.h"
#include <complex.h>

#ifndef PRECISION
#define PRECISION float
#endif

#define SCALE 	10

void array_conj(PRECISION _Complex* buf, unsigned height, unsigned width)
{
	for( unsigned i = 0; i < height; i++ )
	{
		for( unsigned j = 0; j < width; j++ )
		{
			buf[i*height + j] = conjf(buf[i*height + j]);
		}
	}
}

void transpose(PRECISION _Complex* buf, unsigned height, unsigned width)
{
	for( unsigned i = 0; i < height; i++ )
	{
		for( unsigned j = i+1; j < width; j++ )
		{
			if( i != j )
			{
				PRECISION _Complex upper = *(buf + image_height*i + j);
				*(buf + height*i + j) = *(buf + height*j + i);
				*(buf + height*j + i) = upper;
			}
		}
	}
}

void _fft(PRECISION _Complex buf[], PRECISION _Complex out[], int n, int step)
{
	if (step < n) {
		_fft(out, buf, n, step * 2);
		_fft(out + step, buf + step, n, step * 2);
		int i;
		for (i = 0; i < n; i += 2 * step) {
			PRECISION _Complex t1 = (PRECISION) cexp(-I * M_PI * i / n);
		    PRECISION _Complex t  = t1 * out[i + step];
			buf[i / 2]     = out[i] + t;
			buf[(i + n)/2] = out[i] - t;
		}
	}
}

void conv(PRECISION* grey)
{
	// input init
	PRECISION _Complex* complex_grey = (PRECISION _Complex* )malloc(image_height*image_width*sizeof(PRECISION _Complex));
	PRECISION _Complex* fft_0        = (PRECISION _Complex* )malloc(image_height*image_width*sizeof(PRECISION _Complex));
	PRECISION _Complex* fft_1        = (PRECISION _Complex* )malloc(image_height*image_width*sizeof(PRECISION _Complex));
	PRECISION _Complex* fft_2        = (PRECISION _Complex* )malloc(image_height*image_width*sizeof(PRECISION _Complex));
	PRECISION _Complex* fft_3        = (PRECISION _Complex* )malloc(image_height*image_width*sizeof(PRECISION _Complex));
	for( unsigned i = 0; i < image_height; i++ )
	{
		for( unsigned j = 0; j < image_width; j++ )
		{
			complex_grey[image_width*i + j] = grey[image_width*i + j] + I*0.0;
		}
	}
	// 2d fft
	for( unsigned i = 0; i < image_height; i++ )
	{
		_fft(&complex_grey[i*image_width], &fft_0[i*image_width], image_width, 1);
	}
	// transpose
	transpose(fft_0, image_height, image_width);
	for( unsigned i = 0; i < image_width; i++ )
	{
		_fft( &fft_0[i*image_height], &fft_1[i*image_height], image_height, 1);
	}
	// element-wise multiply
	for( unsigned i = 0; i < image_width; i++ )
	{
		for( unsigned j = 0; j < image_height; j++ )
		{
			fft_1[i*image_height + j] = SCALE*fft_1[i*image_height + j];
		}
	}
	// ifft(X) = (1/N) * conj(FFT(conj(X)))
	array_conj(fft_1, image_width, image_height);
	for( unsigned i = 0; i < image_width; i++ )
	{
		_fft( &fft_1[i*image_height], &fft_2[i*image_height], image_height, 1);
		for( unsigned j = 0; j < image_height; j++ )
		{
			fft_2[i*image_height + j] = 1.0 / (PRECISION)image_height * fft_2[i*image_height + j];
		}
	}
	array_conj(fft_2, image_width, image_height);
	transpose(fft_2, image_width, image_height);
	for( unsigned i = 0; i < image_height; i++ )
	{
		_fft(&fft_2[i*image_width], &fft_3[i*image_width], image_width, 1);
		for( unsigned j = 0; j < image_width; j++ )
		{
			fft_2[i*image_width + j] = 1.0 / (PRECISION)image_width * fft_2[i*image_width + j];
		}
	}
	for( unsigned i = 0; i < image_height; i++ )
	{
		for( unsigned j = 0; j < image_width; j++ )
		{
			grey[image_width*i + j] = crealf(fft_3[i*image_width + j]);
		}
	}
	free(complex_grey);
	free(fft_0);
	free(fft_1);
	free(fft_2);
	free(fft_3);
}

int main(int argc, char** argv)
{
	if( argc != 3 )
	{
		printf("Please provide input image file path and output file path.\n");
		return EXIT_FAILURE;
	}
	struct Pixel* input = readImage(argv[1]);
	PRECISION* output   = (PRECISION*)calloc(image_width*image_height, sizeof(PRECISION));

	// convert image to intensity values
	for( unsigned i = 0; i < image_height; i++ )
	{
		for( unsigned int j = 0; j < image_width; j++ )
		{
			*(output + i*image_width + j) += (PRECISION)(input + i*image_width + j)->r / 3.0;
			*(output + i*image_width + j) += (PRECISION)(input + i*image_width + j)->g / 3.0;
			*(output + i*image_width + j) += (PRECISION)(input + i*image_width + j)->b / 3.0;
		}
	}

	__TIMINGLIB_benchmark( [&]{ conv(output); } );

	for( unsigned i = 0; i < image_height; i++ )
	{
		for( unsigned j = 0; j < image_width; j++ )
		{
			(input + image_width*i + j)->r = *(output + image_width*i + j);
			(input + image_width*i + j)->g = *(output + image_width*i + j);
			(input + image_width*i + j)->b = *(output + image_width*i + j);
		}
	}

	writeImage(input, argv[2]);
	free(input);
	free(output);
	return 0;
}
