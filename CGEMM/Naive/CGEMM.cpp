
#include <stdlib.h>
#include <string.h>
#include "TimingLib.h"

#define PRECISION 	float
#ifndef SIZE
#define SIZE 		64
#endif

void read_input(PRECISION* a)
{
	for( unsigned int i = 0; i < SIZE; i++ )
	{
		for( unsigned int j = 0; j < SIZE; j++ )
		{
			*(a + i*SIZE + j) = (PRECISION)rand();
		}
	}
}

void GEMM(PRECISION* a, PRECISION* b, PRECISION* c)
{
	for( unsigned int i = 0; i < SIZE; i++ )
	{
		for( unsigned int j = 0; j < SIZE; j++ )
		{
			for( unsigned int k = 0; k < SIZE; k++ )
			{
				*(c + i*SIZE + j) += *(a + i*SIZE + k) * *(b + k*SIZE + j);
			}
		}
	}
}

void Madd(PRECISION* a, PRECISION* b, PRECISION* c)
{
	for( unsigned int i = 0; i < SIZE; i++ )
	{
		for( unsigned int j = 0; j < SIZE; j++ )
		{
			*(c + i*SIZE + j) = *(a + i*SIZE + j) + *(b + i*SIZE + j);
		}
	}
}

void Msub(PRECISION* a, PRECISION* b, PRECISION* c)
{
	for( unsigned int i = 0; i < SIZE; i++ )
	{
		for( unsigned int j = 0; j < SIZE; j++ )
		{
			*(c + i*SIZE + j) = *(a + i*SIZE + j) - *(b + i*SIZE + j);
		}
	}
}

int main()
{
	PRECISION* a = (PRECISION*)calloc( SIZE*SIZE, sizeof(PRECISION) );
	PRECISION* b = (PRECISION*)calloc( SIZE*SIZE, sizeof(PRECISION) );
	PRECISION* c = (PRECISION*)calloc( SIZE*SIZE, sizeof(PRECISION) );
	PRECISION* d = (PRECISION*)calloc( SIZE*SIZE, sizeof(PRECISION) );
	PRECISION* e = (PRECISION*)calloc( SIZE*SIZE, sizeof(PRECISION) );
	PRECISION* f = (PRECISION*)calloc( SIZE*SIZE, sizeof(PRECISION) );
	PRECISION* scratch = (PRECISION*)calloc( SIZE*SIZE, sizeof(PRECISION) );

	read_input(a);
	read_input(b);
	read_input(c);
	read_input(d);

	__TIMINGLIB_benchmark( [&] {
		// e + jf = (a + jb)(c + jd) = ac - bd + j(bc + ad)
		GEMM(a, c, scratch);
		GEMM(b, d, e);
		Msub(scratch, e, e);
		memset(scratch, 0, SIZE*SIZE*sizeof(PRECISION));
		GEMM(b, c, scratch);
		GEMM(a, d, f);
		Madd(scratch, f, f);
	} );

	free(a);
	free(b);
	free(c);
	free(d);
	free(e);
	free(f);
	free(scratch);
	return 0;
}
