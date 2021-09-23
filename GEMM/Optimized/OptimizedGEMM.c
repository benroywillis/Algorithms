
#include <x86intrin.h>

#define SIZE 		256
#define BLOCKSIZE 	32
#define UNROLL 		(4)

// taken from patterson-hennessy "Computer Organization and Design, RISK-V Edition" chapter 5 section 15 "Going Fast: Exploiting Memory Hierarchy", Figure 5.47 pg 466

void do_block( int n, int si, int sj, int sk, double* A, double* B, double* C )
{
	for( int i = si; i < si + BLOCKSIZE; i += UNROLL*4 )
	{
		for( int j = sj; j < sj + BLOCKSIZE; j++ )
		{
			__m256d c[UNROLL];
			for( int x = 0; x < UNROLL; x++ )
			{
				c[x] = _mm256_load_pd( C + i + x*UNROLL + j*n );
			}
			for( int k = sk; k < sk + BLOCKSIZE; k++ )
			{
				__m256d b = _mm256_broadcast_sd( B + k + j * n );
				for( int x = 0; x < UNROLL; x++ )
				{
					c[x] = _mm256_add_pd( c[x], _mm256_mul_pd( _mm256_load_pd( A + n*k + x*UNROLL + i ), b ) );
				}
			}
			for( int x = 0; x < UNROLL; x++ )
			{
				_mm256_store_pd( C + i + x*UNROLL + j*n, c[x] );
			}
		}
	}
}

int main()
{
	// if we malloc our arrays the _mm256_load_pd() operations segfault
	//double* A = (double* )malloc( SIZE*SIZE*sizeof(double) );
	//double* B = (double* )malloc( SIZE*SIZE*sizeof(double) );
	//double* C = (double* )malloc( SIZE*SIZE*sizeof(double) );
	double A[SIZE][SIZE];
	double B[SIZE][SIZE];
	double C[SIZE][SIZE];
	for( int i = 0; i < SIZE; i += BLOCKSIZE )
	{
		for( int j = 0; j < SIZE; j += BLOCKSIZE )
		{
			for( int k = 0; k < SIZE; k += BLOCKSIZE )
			{
				do_block( SIZE, i, j, k, (double*)A, (double*)B, (double*)C );
			}
		}
	}
	
	return 0;
}
