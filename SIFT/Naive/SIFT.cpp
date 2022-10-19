
// taken from github.com/victoroliv2/halide-casestudies/blob/master/sift.cpp

#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <stdalign.h>
#include <omp.h>
#include <string.h>
#include <float.h>
#include "BilateralFilter.h"
#include "TimingLib.h"

inline static int clamp(int x, int low, int high)
{
  return (x < low) ? low : ((x > high) ? high : x);
}

inline static int max(int a, int b)
{
  return (a < b) ? b : a;
}

inline static int min(int a, int b)
{
  return (a < b) ? a : b;
}

#include <stdio.h>

#define SIGMA 1.6f

void
sift (float const * __restrict__ _input,
      unsigned char * __restrict__ _output,
      int    width,
      int    height,
      int    octaves,
      int    intervals,
      float  curv_thr,
      float  contr_thr)
{
  const int channels = 4;

  float * __restrict__ input = (float * __restrict__) __builtin_assume_aligned(_input,  32);
  unsigned char * __restrict__ output = (unsigned char * __restrict__) __builtin_assume_aligned(_output, 32);

  #define INPUT(x,y,c) input[c+channels*(x + width * y)]
  #define OUTPUT(x,y)  output[x + width * y]

  memset(output, 0, width*height*sizeof(unsigned char));

  float sig[intervals+3], sig_prev, sig_total;

  sig[0] = SIGMA;
  float p = pow( 2.0, 1.0 / intervals );
  for(int i = 1; i < intervals + 3; i++ )
    {
      sig_prev = powf( p, i - 1 ) * SIGMA;
      sig_total = sig_prev * p;
      sig[i] = sqrtf( sig_total * sig_total - sig_prev * sig_prev );
    }

  float * __restrict__ gauss_pyr[octaves][intervals+3];

  #define  GAUSS_PYR(o,i,x,y) gauss_pyr[o][i][y*W+x]

  for(int o = 0; o < octaves; o++ )
    for(int i = 0; i < intervals + 3; i++ )
      {
        const int W = width  >> o;
        const int H = height >> o;

        posix_memalign((void **) &gauss_pyr[o][i], 32, W * H * sizeof(float));

        if( o == 0  &&  i == 0 ) {

          #pragma omp parallel for
          for(int y=0; y<H; y++)
            for(int x=0; x<W; x++)
              GAUSS_PYR(o,i,x,y) = 0.299f * INPUT(x, y, 0) + 0.587f * INPUT(x, y, 1) + 0.114f * INPUT(x, y, 2);
        }
        /* base of new octvave is halved image from end of previous octave */
        else if( i == 0 ) {
          float * __restrict__ down;

          posix_memalign((void **) &down, 32, 2 * W * H * sizeof(float));

          #define DOWN(x,y) down[x + W * y]

          #pragma omp parallel for
          for(int y=0; y<2*H; y++)
            for(int x=0; x<W; x++)
              DOWN(x, y) = (GAUSS_PYR(o-1,i, max(2*x-1, 0), y) +
                            3.0f * (GAUSS_PYR(o-1,i, 2*x, y) + GAUSS_PYR(o-1,i, min(2*x+1, 2*W-1), y)) +
                            GAUSS_PYR(o-1,i, min(2*x+2, 2*W-1), y)) / 8.0f;

          #pragma omp parallel for
          for(int y=0; y<H; y++)
            for(int x=0; x<W; x++)
              GAUSS_PYR(o,i,x,y) = (DOWN(x, max(2*y-1, 0)) +
                                    3.0f * (DOWN(x, 2*y) + DOWN(x, min(2*y+1, H-1))) +
                                    DOWN(x, min(2*y+2, H-1))) / 8.0f;

          free(down);
        }
        /* blur the current octave's last image to create the next one */
        else {
          float * __restrict__ blur;

          posix_memalign((void **) &blur, 32, W * H * sizeof(float));

          #define BLUR(x,y) blur[x + W * y]

          float sum = 0.0f;

          const int radius = int(3*sig[i] + 1.0f);

          float gaussian_mask[2 * radius + 1];

          for (int i=-radius; i<=radius; i++)
            sum += gaussian_mask[i+radius] = exp(-(float(i)/sig[i])*(float(i)/sig[i])*0.5f);

          for (int i=-radius; i<=radius; i++)
            gaussian_mask[i+radius] /= sum;

          #pragma omp parallel for
          for(int y=0; y<H; y++)
            for(int x=0; x<W; x++) {
              float v = 0.0f;

              for(int r=-radius; r <= radius; r++)
                v += gaussian_mask[r+radius] * GAUSS_PYR(o,i-1,clamp(x+r, 0, W-1),clamp(y+r, 0, H-1));

              BLUR(x,y) = v;
            }

          #pragma omp parallel for
          for(int y=0; y<H; y++)
            for(int x=0; x<W; x++) {
              float v = 0.0f;

              for(int r=-radius; r <= radius; r++)
                v += gaussian_mask[r+radius] * BLUR(clamp(x+r, 0, W-1),clamp(y+r, 0, H-1));

              GAUSS_PYR(o,i,x,y) = v;
            }

          free(blur);
        }
      }

  float * __restrict__ dog_pyr[octaves][intervals+2];

  #define  DOG_PYR(o,i,x,y) dog_pyr[o][i][y*W+x]

  for(int o = 0; o < octaves; o++ )
    for(int i = 0; i < intervals + 2; i++ ) {
      const int W = width  >> o;
      const int H = height >> o;

      posix_memalign((void **) &dog_pyr[o][i], 32, W * H * sizeof(float));

      #pragma omp parallel for
      for(int y=0; y<H; y++)
        for(int x=0; x<W; x++)
          DOG_PYR(o,i,x,y) = GAUSS_PYR(o,i+1,x,y) - GAUSS_PYR(o,i,x,y);
    }

  for(int o = 0; o < octaves; o++ )
    for(int i = 1; i <= intervals; i++ )
      {
        const int W = width  >> o;
        const int H = height >> o;

        #pragma omp parallel for
        for(int y=0; y<H; y++)
          for(int x=0; x<W; x++)
            {
              float v[3][3][3];

              for (int ry=-1; ry<=1; ry++)
                for (int rx=-1; rx<=1; rx++)
                  {
                    v[0][ry+1][rx+1] = DOG_PYR(o, i-1, clamp(x+rx, 0, W-1), clamp(y+ry, 0, H-1));
                    v[1][ry+1][rx+1] = DOG_PYR(o, i,   clamp(x+rx, 0, W-1), clamp(y+ry, 0, H-1));
                    v[2][ry+1][rx+1] = DOG_PYR(o, i+1, clamp(x+rx, 0, W-1), clamp(y+ry, 0, H-1));
                  }

              const float vcc = v[1][1][1];

              float dmax = -FLT_MAX;
              float dmin =  FLT_MAX;

              for (int ry=0; ry<=2; ry++)
                for (int rx=0; rx<=2; rx++)
                  {
                    dmax = fmax(dmax,
                           fmax(v[0][ry][rx],
                           fmax(v[1][ry][rx],
                                v[2][ry][rx])));

                    dmin = fmin(dmin,
                           fmin(v[0][ry][rx],
                           fmin(v[1][ry][rx],
                                v[2][ry][rx])));
                  }

              const float prelim_contr_thr = 0.5f * contr_thr / intervals;

              const bool is_extremum = ((fabs(vcc) > prelim_contr_thr) &&
                                        ((vcc <= 0.0f && vcc == dmin) ||
                                         (vcc >  0.0f && vcc == dmax)));

              const float dxx = v[1][1][2] + v[1][1][0] - 2.0f * vcc;
              const float dyy = v[1][2][1] + v[1][0][1] - 2.0f * vcc;
              const float dss = v[2][1][1] + v[0][1][1] - 2.0f * vcc;
              const float dxy = ( v[1][2][2] - v[1][2][0] - v[1][0][2] + v[1][0][0] ) / 4.0f;
              const float dxs = ( v[2][1][2] - v[2][1][0] - v[0][1][2] + v[0][1][0] ) / 4.0f;
              const float dys = ( v[2][2][1] - v[2][0][1] - v[0][2][1] + v[0][0][1] ) / 4.0f;

              const float pc_det = dxx * dyy - 2.0f * dxy;
              const float pc_tr = dxx + dyy;

              float invdet = 1.0f/(  ( dxx * (dyy * dss - dys * dys) )
                                   - ( dxy * (dxy * dss - dys * dxs) )
                                   + ( dxs * (dxy * dys - dyy * dxs) ));

              const float inv_dxx = invdet * (dyy * dss - dys * dys);
              const float inv_dyy = invdet * (dxx * dss - dxs * dxs);
              const float inv_dss = invdet * (dxx * dyy - dxy * dxy);
              const float inv_dxy = invdet * (dxs * dys - dxy * dss);
              const float inv_dxs = invdet * (dxy * dys - dxs * dyy);
              const float inv_dys = invdet * (dxy * dxs - dxx * dys);

              const float dx = (v[1][1][2] - v[1][1][0]) / 2.0f;
              const float dy = (v[1][2][1] - v[1][0][1]) / 2.0f;
              const float ds = (v[2][1][1] - v[0][1][1]) / 2.0f;

              const float interp_x = inv_dxx * dx + inv_dxy * dy + inv_dxs * ds;
              const float interp_y = inv_dxy * dx + inv_dyy * dy + inv_dys * ds;
              const float interp_s = inv_dxs * dx + inv_dys * dy + inv_dss * ds;

              const float interp_contr = interp_x * dx + interp_y * dy + interp_s * ds;

              bool ok = is_extremum &&
                        pc_det > 0.0f;
                        (pc_tr * pc_tr / pc_det < ( curv_thr + 1.0f )*( curv_thr + 1.0f ) / curv_thr) &&
                        fabs(interp_contr) > contr_thr / intervals &&
                        dx < 1.0f &&
                        dy < 1.0f &&
                        ds < 1.0f;

              if (ok) OUTPUT(x << o, y << o) = 1;
            }
      }

  for(int o = 0; o < octaves; o++ )
    for(int i = 0; i < intervals + 3; i++ )
      free(gauss_pyr[o][i]);

  for(int o = 0; o < octaves; o++ )
    for(int i = 0; i < intervals + 2; i++ )
      free(dog_pyr[o][i]);
}

int main(int argc, char** argv)
{
	if( argc != 7 )
	{
		printf("Please provide octaves, intervals, curve threshold, contrast threshold, input image path and output image path.\n");
		return EXIT_FAILURE;
	}
	int octaves     = atoi(argv[1]);
	int intervals   = atoi(argv[2]);
	float curve_thr = strtof(argv[3], NULL);
	float contr_thr = strtof(argv[4], NULL);
	struct Pixel* input  = readImage(argv[5]);
	struct Pixel* output = (struct Pixel*)calloc(image_width*image_height, sizeof(struct Pixel));
	printf("Image size: %d x %d\n", image_height, image_width);

	// convert to floating point grayscale
	int num_channels = 3;
	float* in = (float*)aligned_alloc(32, image_height*image_width*num_channels*sizeof(float));
	for( unsigned y = 0; y < image_height; y++ )
	{
		for( unsigned x = 0; x < image_width; x++ )
		{
			for( unsigned c = 0; c < num_channels; c++ )
			{
				if( c == 0 )
				{
					in[y*image_width*num_channels + x*num_channels + c] = (float)input[y*image_width + x].r;
				}
				else if( c == 1 )
				{
					in[y*image_width*num_channels + x*num_channels + c] = (float)input[y*image_width + x].g;
				}
				else
				{
					in[y*image_width*num_channels + x*num_channels + c] = (float)input[y*image_width + x].b;
				} 
			}
		}
	}

	uint8_t* out = (uint8_t*)aligned_alloc(32, image_height*image_width*sizeof(uint8_t));
	__TIMINGLIB_benchmark( [&]() { sift(in, out, image_width, image_height, octaves, intervals, curve_thr, contr_thr); } );

	// output pixels are either 1 or 0, so multiply everything by 255 to highlight the 1 pixels
	for( unsigned y = 0; y < image_height; y++ )
	{
		for( unsigned x = 0; x < image_width; x++ )
		{
			output[y*image_width + x].r = out[y*image_width + x]*255;
			output[y*image_width + x].g = out[y*image_width + x]*255;
			output[y*image_width + x].b = out[y*image_width + x]*255;
			/*if( out[y*image_width + x] )
			{
				printf("Non-zero pixel at %d,%d\n", y, x);
			}*/
		}
	}

	writeImage(output, argv[6]);

	free(input);
	free(output);
	free(in);
	free(out);
	return 0;
}
