
#include <stdlib.h>
#include "TimingLib.h"

#ifndef PRECISION
#define TYPE float
#elif PRECISION == 0
#define TYPE float
#elif PRECISION == 1
#define TYPE double
#elif PRECISION == 2
#define TYPE int
#elif PRECISION == 3
#define TYPE long
#elif PRECISION == 4
#define TYPE unsigned int
#elif PRECISION == 5
#define TYPE unsigned long
#else
#define TYPE double
#endif

#ifndef SIZE
#define SIZE 64
#endif

int main() 
{
	TYPE* A = (TYPE*)malloc(SIZE*sizeof(TYPE));
	TYPE* B = (TYPE*)malloc(SIZE*sizeof(TYPE));
	TYPE* C = (TYPE*)malloc(SIZE*SIZE*sizeof(TYPE));
	for( unsigned i = 0; i < SIZE; i++ ) {
		A[i] = (TYPE)rand();
		B[i] = (TYPE)rand();
		for( unsigned j = 0; j < SIZE; j++ ) {
			C[i*SIZE+j] = (TYPE)0;
		}
	}
	__TIMINGLIB_benchmark( [&]{
		// tensor multiply in our example is going to be between 1D tensors (vectors)
		// thus, we implement outer product
		for( unsigned i = 0; i < SIZE; i++ ) {
			for( unsigned j = 0; j < SIZE; j++ ) {
				C[i*SIZE+j] = A[i]*B[j];
			}
		}
	});
	return 0;
}
