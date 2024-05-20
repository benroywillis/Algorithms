
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "TimingLib.h"


#ifndef PRECISION
#define TYPE float
#elif   PRECISION == 0
#define TYPE float
#elif   PRECISION == 1
#define TYPE double
#endif

#ifndef SIZE
#define SIZE 		64
#endif

#define L	5

// implements 1d max pooling on a 3d input
// stride length is the filter length
void maxpool(TYPE *in, TYPE *out)
{
#pragma scop
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            for (int k = 0; k < SIZE; k += L) {
				TYPE max = (TYPE)0;
				for( int l = 0; l < L; l++ ) {
					if( in[i*SIZE*SIZE + j*SIZE + k + l] > max ) max = in[i*SIZE*SIZE + j*SIZE + k + l];
				}
				out[i*SIZE*(SIZE/L) + j*(SIZE/L) + k/L] = max;
            }
        }
    }
#pragma endscop
}

int main()
{
    TYPE *in = (TYPE *)malloc(SIZE*SIZE*SIZE*sizeof(TYPE));
    TYPE *out = (TYPE *)malloc(SIZE*SIZE*(SIZE/L+1)*sizeof(TYPE));

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
        	for (int k = 0; k < SIZE; k++) {
            	in[i*SIZE+j] = rand();
            	out[i*SIZE+j] = 0;
			}
        }
    }

	__TIMINGLIB_benchmark([&]{ maxpool(in, out); });

	// to make the optimizer preserve the maxpool
	volatile bool is = out[0];

    return 0;
}
