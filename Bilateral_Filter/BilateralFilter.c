#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
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
};

void readImage(struct Pixel* in, char* file)
{
	// reference: instesre.org/howto/BW_image/ReadingBitmaps.htm
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
	fread(&height, sizeof(uint8_t), 4, f);
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

	in = (struct Pixel*)malloc( width*height*sizeof(struct Pixel) );
	//output = (PRECISION*)malloc( width*height*sizeof(PRECISION) );
	// 3. read image data
	// it is all encoded in 3-word rgb data
	uint8_t newPixel[3];
	for( unsigned int i = 0; i < height; i++ )
	{
		for( unsigned int j = 0; j < width; j++ )
		{
			fread(&newPixel, sizeof(uint8_t), 3, f);
			(in + i*height+ j)->r = newPixel[0];
			(in + i*height+ j)->g = newPixel[1];
			(in + i*height+ j)->b = newPixel[2];
		}
		// read "end-of-line" mark between each line
		fread(&buffer, sizeof(uint8_t), 1, f);
	}
	printf("Image width, height, size: %d, %d, %d\n", width, height, imageSize);
	for( unsigned int i = 0; i < height; i++ )
	{
		for( unsigned int j = 0; j < width; j++ )
		{
			//(in + i*SIZE + j)->r = (uint8_t)rand();
			//(in + i*SIZE + j)->g = (uint8_t)rand();
			//(in + i*SIZE + j)->b = (uint8_t)rand();
			printf("%d, %d, %d | ", (in + i*height+ j)->r, (in + i*height+ j)->g, (in + i*height+ j)->b);
		}
		printf("\n");
	}
	exit(0);
}

PRECISION norm_space(struct Pixel* p, int x0, int y0, struct Pixel* q, int x1, int y1)
{
	PRECISION sum = 0;
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
	// this distribution is N( 0, SIGMA_S )
	static PRECISION s_2 = SIGMA_S * SIGMA_S;
	return ( 1 / (2*M_PI * s_2) )*exp( (-1/2) * (diff*diff / 2*s_2) );
}

// computes the range component
PRECISION g(PRECISION diff)
{
	// this distribution is N( 0, SIGMA_R )
	static PRECISION r_2 = SIGMA_R * SIGMA_R;
	return ( 1 / (2*M_PI * r_2) )*exp( (-1/2) * ((diff)*(diff) / 2*r_2) );
}

void BilateralFilter(struct Pixel* in, PRECISION* out)
{
	// for each pixel
	for( unsigned int i = K/2; i < SIZE-(K/2); i++ )
	{
		for( unsigned int j = L/2; j < SIZE-(L/2); j++ )
		{
			struct Pixel* cP = (in + i*SIZE + j);
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
					PRECISION d = norm_space( cP, i, j, cP + k*SIZE + l, i+k, j+l );
					// pixel intensity difference
					// since we have an input grayscale image these two values are the same
					PRECISION d_i = fabs( Intensity(cP + k*SIZE + l) - Intensity(cP) );
					PRECISION f_out = f(d);
					PRECISION g_out = g(d_i);
					PRECISION mul = f_out * g_out;
					*(out + i*SIZE + j) = mul*Intensity(cP + k*SIZE + l);
					Wp += mul;
				}
			}
			*(out + i*SIZE + j) /= Wp;
		}
	}
}

int main(int argc, char** argv)
{
	struct Pixel* input;
	PRECISION* output;
	// the input space is rgb with 3-word width
	if( argc > 1 )
	{
		readImage(input, argv[1]);
	}
	else
	{
		readImage(input, "john.bmp");
	}
	// the output space is grayscale
	char* outputName;
	if( argc > 2 )
	{
		outputName = argv[2];
	}
	else
	{
		outputName = "output.bmp";
	}

	struct timespec start;
	struct timespec end;
	while(clock_gettime(CLOCK_MONOTONIC, &start));
	BilateralFilter(input, output);
	while(clock_gettime(CLOCK_MONOTONIC, &end));
	double secdiff = (double)(end.tv_sec - start.tv_sec);
	double nsecdiff = (double)(end.tv_nsec - start.tv_nsec) * pow(10.0, -9.0);
	double totalTime = secdiff + nsecdiff;
	printf("Bilateral filter runtime: %fs\n", totalTime);

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
