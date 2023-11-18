
#include <stdlib.h>
#include <string.h>
#include "TimingLib.h"
#include <complex.h>

#define PRECISION 	float
#ifndef SIZE
#define SIZE 		64
#endif
/* this is the non-fused CGEMM implementation
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
}*/

//_Complex PRECISION *ab;
//_Complex PRECISION *cd;
//_Complex PRECISION *ef;

int main()
{
	/*PRECISION* a = (PRECISION*)calloc( SIZE*SIZE, sizeof(PRECISION) );
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
	} );*/

	// now do a fused implementation and see if the answer is the same
	_Complex PRECISION* ab = (_Complex PRECISION*)calloc( SIZE*SIZE, sizeof(_Complex PRECISION) );
	_Complex PRECISION* cd = (_Complex PRECISION*)calloc( SIZE*SIZE, sizeof(_Complex PRECISION) );
	_Complex PRECISION* ef = (_Complex PRECISION*)calloc( SIZE*SIZE, sizeof(_Complex PRECISION) );
	//ab = (_Complex PRECISION*)calloc( SIZE*SIZE, sizeof(_Complex PRECISION) );
	//cd = (_Complex PRECISION*)calloc( SIZE*SIZE, sizeof(_Complex PRECISION) );
	//ef = (_Complex PRECISION*)calloc( SIZE*SIZE, sizeof(_Complex PRECISION) );
	for( unsigned i = 0; i < SIZE; i++ )
	{
		for( unsigned j = 0; j < SIZE; j++ )
		{
			//ab[i*SIZE+j] = a[i*SIZE+j] + b[i*SIZE+j]*I;
			//cd[i*SIZE+j] = c[i*SIZE+j] + d[i*SIZE+j]*I;
			ab[i*SIZE+j] = rand() + rand()*I;
			cd[i*SIZE+j] = rand() + rand()*I;
		}
	}
	__TIMINGLIB_benchmark( [&] {
		for( unsigned i = 0; i < SIZE; i++ )
		{
			for( unsigned j = 0; j < SIZE; j++ )
			{
				for( unsigned k = 0; k < SIZE; k++ )
				{
					ef[i*SIZE+j] += ab[i*SIZE+k] * cd[k*SIZE+j];
				}
			}
		}
	} );

	/* comparison between unfused naive version and fused version
	_Complex double diff  = 0.0 + 0.0*I;
	_Complex double total = 0.0 + 0.0*I;
	for( unsigned i = 0; i < SIZE; i++ )
	{
		for( unsigned j = 0; j < SIZE; j++ )
		{
			diff  += cabs(ef[i*SIZE+j] - (e[i*SIZE+j] + f[i*SIZE+j]*I));
			total += ef[i*SIZE+j] - (e[i*SIZE+j] + f[i*SIZE+j]*I);
		}
	}
	printf("Answer difference: %g%%\n", creal(diff / total) * 100);*/

	/*free(a);
	free(b);
	free(c);
	free(d);
	free(e);
	free(f);
	free(scratch);*/
	free(ab);
	free(cd);
	free(ef);
	return 0;
}
