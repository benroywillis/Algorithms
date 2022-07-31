
#ifndef BILATERALFILTER_H
#define BILATERALFILTER_H

#include <stdint.h>

#ifndef PRECISION
#define PRECISION 	double
#endif

#ifndef K
#define K			5
#endif

#ifndef L
#define L			5
#endif

// filter parameters
extern float sigma_s;
extern float sigma_r;
// image dimensions
extern uint32_t image_width;
extern uint32_t image_height;

struct Pixel
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

struct Pixel* readImage(char* file);

void writeImage(struct Pixel* out, char* file);

PRECISION norm_space(struct Pixel* p, int x0, int y0, struct Pixel* q, int x1, int y1);

// converts rgb to grayscale
PRECISION Intensity(struct Pixel* a);

// computes the spacial component
PRECISION f(PRECISION diff);

// computes the range component
PRECISION g(PRECISION diff);

void BilateralFilter(struct Pixel* in, PRECISION* out);

#endif
