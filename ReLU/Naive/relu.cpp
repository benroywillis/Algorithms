#include <stdlib.h>
#include "TimingLib.h"

#ifndef PRECISION
#define TYPE float
#elif PRECISION == 0
#define TYPE float
#elif PRECISION == 1
#define TYPE double
#else
#define TYPE double
#endif

#ifndef ROWS
#define ROWS 512
#endif
#ifndef COLS
#define COLS 128
#endif

void ReLU( TYPE* A, TYPE* B )
{
	for( int i = 0; i < ROWS; i++ ) {
		for( int j = 0; j < COLS; j++ ) {
			if( A[i*COLS+j] < (TYPE)0) {
				B[i*COLS+j] = (TYPE)0;
			}
			else {
				B[i*COLS+j] = A[i*COLS+j];
			}
		}
	}
}

int main() {
	TYPE* A = (TYPE*)malloc(sizeof(TYPE)*ROWS*COLS);
	TYPE* B = (TYPE*)malloc(sizeof(TYPE)*ROWS*COLS);
	for( int i = 0; i < ROWS; i++ ) {
		for( int j = 0; j < COLS; j++ ) {
			A[i*COLS+j] = rand()-RAND_MAX/2;
		}
	}
	__TIMINGLIB_benchmark( [&]{
		ReLU(A, B);
	});
	free(A);
	free(B);
	return 0;
}
