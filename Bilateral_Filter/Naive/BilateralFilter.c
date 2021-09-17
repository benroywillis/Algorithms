#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define PRECISION 	double
#define K			5
#define L			5

// filter parameters
float sigma_s;
float sigma_r;
// image dimensions
uint32_t image_width  = 0;
uint32_t image_height = 0;

struct Pixel
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

struct Pixel* readImage(char* file)
{
	// reference: instesre.org/howto/BW_image/ReadingBitmaps.htm
	// reference2: github.com/marc-q/libbmp
	// reading in bmp files requires 4 parts
	// 1. read header: basic file information 14 bytes long
 	//    2 bytes: BM, represented in ASCII code 66 and 77
	//    4 bytes: file size in bytes
	//    4 bytes: two two-byte "reserved values" that are not needed
	//    4 bytes: offset to the beginning of image data, in bytes
	FILE* f = fopen(file, "rb");
	if( !f )
	{
		printf("Failed to open %s!\n", file);
		exit(1);
	}
	uint8_t buffer[4];
	uint16_t reserve0;
	uint16_t reserve1;
	uint32_t fileSize;
	uint32_t offset;
	// don't care about the first two, use fileSize as a buffer
	fread(&buffer, sizeof(uint8_t), 1, f);
	fread(&buffer, sizeof(uint8_t), 1, f);
	// read fileSize
	memset(&buffer, 0, 4*sizeof(uint8_t));
	fread(&buffer, sizeof(uint8_t), 4, f);
	fileSize = (uint32_t)buffer[3]*16777216+(uint32_t)buffer[2]*65536+(uint32_t)buffer[1]*256+(uint32_t)buffer[0];
	// don't care about reserved values, use offset as a buffer
	fread(&reserve0, sizeof(uint16_t), 1, f);
	fread(&reserve1, sizeof(uint16_t), 1, f);
	// read offset
	memset(&buffer, 0, 4*sizeof(uint8_t));
	fread(&buffer, sizeof(uint8_t), 4, f);
	offset = (uint32_t)buffer[3]*16777216+(uint32_t)buffer[2]*65536+(uint32_t)buffer[1]*256+(uint32_t)buffer[0];
	// 2. read image information
	//    4 bytes: Header size, in bytes (should be 40)
	//    4 bytes: Image width, in pixels
	//    4 bytes: Image height, in pixels
	//    2 bytes: Number of color planes
	//    2 bytes: Bits per pixel, 1 to 24
	//    4 bytes: Compression, bytes (I assume it is 0)
	//    4 bytes: Image size, bytes
	//    4 bytes each: X-resolution and y-resolution, pixels per meter
	//    4 bytes each: Number of colors and "important colors"
	uint32_t headerSize;
	uint32_t width;
	uint32_t height;
	uint16_t colorPlanes;
	uint16_t bitsPerPixel;
	uint32_t compression;
	uint32_t imageSize;
	uint32_t XResolution;
	uint32_t YResolution;
	uint32_t importantColors;
	fread(&headerSize, sizeof(uint8_t), 4, f);
	fread(&width, sizeof(uint8_t), 4, f);
	image_width = width;
	fread(&height, sizeof(uint8_t), 4, f);
	image_height = height;
	fread(&colorPlanes, sizeof(uint16_t), 1, f);
	fread(&bitsPerPixel, sizeof(uint16_t), 1, f);
	fread(&compression, sizeof(uint32_t), 1, f);
	fread(&imageSize, sizeof(uint32_t), 1, f);
	fread(&XResolution, sizeof(uint32_t), 1, f);
	fread(&YResolution, sizeof(uint32_t), 1, f);
	for( unsigned int i = 47; i < offset+1; i+=4 )
	{
		fread(&importantColors, sizeof(uint32_t), 1, f);
	}

	struct Pixel* in = (struct Pixel*)malloc( image_width*image_height*sizeof(struct Pixel) );
	//output = (PRECISION*)malloc( width*height*sizeof(PRECISION) );
	// 3. read image data
	// it is all encoded in 3-word rgb data
	fseek(f, (long)offset+1, SEEK_SET);
	uint8_t newPixel[3];
	for( unsigned int i = 0; i < image_height; i++ )
	{
		for( unsigned int j = 0; j < image_width; j++ )
		{
			fread(&newPixel, sizeof(uint8_t), 3, f);
			(in + i*image_height + j)->r = newPixel[0];
			(in + i*image_height + j)->g = newPixel[1];
			(in + i*image_height + j)->b = newPixel[2];
			if( bitsPerPixel > 24 ) 
			{
				fread(&buffer[0], sizeof(uint8_t), (bitsPerPixel-24)/8, f);
			}
		}
		// read "end-of-line" mark between each line
		fread(&buffer, sizeof(uint8_t), image_width % 4, f);
	}
	fclose(f);
	return in;
}

void writeImage(struct Pixel* out, char* file)
{
	// reference: instesre.org/howto/BW_image/ReadingBitmaps.htm
	// reference2: github.com/marc-q/libbmp
	// for notes on what all these operations mean see the read function
	FILE* f = fopen(file, "wb");
	uint8_t buffer[4];
	uint16_t reserve0;
	uint16_t reserve1;
	// filesize is the header plus data
	// this will just be the size of the data plus 54 bytes for the header and file into (14+40)
	uint32_t fileSize = (sizeof(struct Pixel)*image_width + image_width % 4) *image_height + 54;
	// header:14, image info:40
	uint32_t offset = 54;
	// first two numbers are 66 and 77: "BM" in ASCII
	buffer[0] = 66;
	fwrite(&buffer, sizeof(uint8_t), 1, f);
	buffer[0] = 77;
	fwrite(&buffer, sizeof(uint8_t), 1, f);
	// write fileSize from lsbyte to msbyte, left to right
	buffer[0] = (uint8_t)(fileSize & 0xFF);
	buffer[1] = (uint8_t)(fileSize & 0xFF00);
	buffer[2] = (uint8_t)(fileSize & 0xFF0000);
	buffer[3] = (uint8_t)(fileSize & 0xFF000000);
	fwrite(&buffer, sizeof(uint8_t), 4, f);
	// don't care about reserved values
	fwrite(&buffer, sizeof(uint16_t), 1, f);
	fwrite(&buffer, sizeof(uint16_t), 1, f);
	// write offset
	memset(&buffer, 0, 4*sizeof(uint8_t));
	buffer[0] = (uint8_t)offset;
	fwrite(&buffer[0], sizeof(uint8_t), 4, f);
	uint32_t headerSize = 40;
	uint32_t width  = image_width;
	uint32_t height = image_height;
	uint16_t colorPlanes = 1;
	uint16_t bitsPerPixel = 24;
	uint32_t compression = 0;
	uint32_t imageSize = 0;
	uint32_t XResolution = 0;
	uint32_t YResolution = 0;
	uint32_t importantColors = 0;
	fwrite(&headerSize, sizeof(uint32_t), 1, f);
	fwrite(&width, sizeof(uint32_t), 1, f);
	fwrite(&height, sizeof(uint32_t), 1, f);
	fwrite(&colorPlanes, sizeof(uint16_t), 1, f);
	fwrite(&bitsPerPixel, sizeof(uint16_t), 1, f);
	fwrite(&compression, sizeof(uint32_t), 1, f);
	fwrite(&imageSize, sizeof(uint32_t), 1, f);
	fwrite(&XResolution, sizeof(uint32_t), 1, f);
	fwrite(&YResolution, sizeof(uint32_t), 1, f);
	fwrite(&importantColors, sizeof(uint32_t), 1, f);
	fwrite(&importantColors, sizeof(uint32_t), 1, f);

	struct Pixel newPixel;
	for( unsigned int i = 0; i < image_height; i++ )
	{
		for( unsigned int j = 0; j < image_width; j++ )
		{
			newPixel.r = (out + i*image_height + j)->r;
			newPixel.g = (out + i*image_height + j)->g;
			newPixel.b = (out + i*image_height + j)->b;
			fwrite(&newPixel.r, sizeof(uint8_t), 1, f);
			fwrite(&newPixel.g, sizeof(uint8_t), 1, f);
			fwrite(&newPixel.b, sizeof(uint8_t), 1, f);
		}
		// write "end-of-line" mark between each line
		newPixel.r = 10; // newline character
		newPixel.g = 10; // newline character
		newPixel.b = 10; // newline character
		fwrite(&newPixel.r, sizeof(uint8_t), image_width % 4, f);
	}
	fclose(f);
}

PRECISION norm_space(struct Pixel* p, int x0, int y0, struct Pixel* q, int x1, int y1)
{
	PRECISION sum = 0.0;
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
	// this distribution is N( 0, s_2)
	PRECISION s_2 = sigma_s * sigma_s;
	return ( 1.0 / pow((2.0f*M_PI * s_2), 0.5) )*exp( (-1.0/2.0) * (diff*diff / 2.0*s_2) );
}

// computes the range component
PRECISION g(PRECISION diff)
{
	// this distribution is N( 0, r_2)
	PRECISION r_2 = sigma_r * sigma_r;
	return ( 1.0 / pow((2.0*M_PI * r_2), 0.5) )*exp( (-1.0/2.0) * ((diff)*(diff) / 2.0*r_2) );
}

void BilateralFilter(struct Pixel* in, PRECISION* out)
{
	// for each pixel
	for( unsigned int i = K/2; i < image_height-(K/2); i++ )
	{
		for( unsigned int j = L/2; j < image_width-(L/2); j++ )
		{
			struct Pixel* cP = (in + i*image_height + j);
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
					PRECISION d = norm_space( cP, i, j, cP + k*(int)image_height + l, i+k, j+l );
					// pixel intensity difference
					// since we have an input grayscale image these two values are the same
					PRECISION d_i = fabs( Intensity(cP + k*(int)image_height + l) - Intensity(cP) );
					PRECISION f_out = f(d);
					PRECISION g_out = g(d_i);
					PRECISION mul = f_out * g_out;
					*(out + (int)i*(int)image_height + (int)j) += mul*Intensity(cP + k*(int)image_height + l);
					Wp += mul;
				}
			}
			*(out + i*image_height + j) /= Wp;
		}
	}
}

int main(int argc, char** argv)
{
	if( argc !=5 )
	{
		printf("Please provide spacial variance sigma_s, range variance sigma_r, input image name, and output image name\n");
		return 1;
	}
	sigma_s = atoi(argv[1]);
	sigma_r = atoi(argv[2]);
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
