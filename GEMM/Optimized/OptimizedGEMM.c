#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <x86intrin.h>
#include <omp.h>

// 0 for float, 1 for double
#ifndef PRECISION
#define PRECISION 0
#endif
#if PRECISION == 0
#define VECTOR_TYPE __m256
#define TYPE float
#else
#define VECTOR_TYPE __m256d
#define TYPE double
#endif

#ifndef SIZE
#define SIZE 		512
#endif
#if PRECISION == 0
#define BLOCKSIZE 	64
#define UNROLL 		(8)
#else
#define BLOCKSIZE 	32
#define UNROLL 		(4)
#endif

// taken from patterson-hennessy "Computer Organization and Design, RISK-V Edition" chapter 5 section 15 "Going Fast: Exploiting Memory Hierarchy", Figure 5.47 pg 466

void do_block( int n, int si, int sj, int sk, TYPE* A, TYPE* B, TYPE* C )
{
	static VECTOR_TYPE c[UNROLL];
	for( int i = si; i < si + BLOCKSIZE; i += UNROLL*UNROLL )
	{
		for( int j = sj; j < sj + BLOCKSIZE; j++ )
		{
			for( int x = 0; x < UNROLL; x++ )
			{
#if PRECISION == 0
				*(c + x) = _mm256_load_ps( C + i + x*UNROLL + j*n );
#else
				*(c + x) = _mm256_load_pd( C + i + x*UNROLL + j*n );
#endif
			}
			for( int k = sk; k < sk + BLOCKSIZE; k++ )
			{
#if PRECISION == 0
				VECTOR_TYPE b = _mm256_broadcast_ss( B + k + j * n );
#else
				VECTOR_TYPE b = _mm256_broadcast_sd( B + k + j * n );
#endif
				for( int x = 0; x < UNROLL; x++ )
				{
#if PRECISION == 0
					c[x] = _mm256_add_ps( c[x], _mm256_mul_ps( _mm256_load_ps( A + n*k + x*UNROLL + i ), b ) );
#else
					c[x] = _mm256_add_pd( c[x], _mm256_mul_pd( _mm256_load_pd( A + n*k + x*UNROLL + i ), b ) );
#endif
				}
			}
			for( int x = 0; x < UNROLL; x++ )
			{
#if PRECISION == 0
				_mm256_store_ps( C + i + x*UNROLL + j*n, c[x] );
#else
				_mm256_store_pd( C + i + x*UNROLL + j*n, c[x] );
#endif
			}
		}
	}
}

int main()
{
	// if we malloc our arrays the _mm256_load_pd() operations segfault
	TYPE* A = (TYPE*)aligned_alloc( BLOCKSIZE, SIZE*SIZE*sizeof(TYPE) );
	TYPE* B = (TYPE*)aligned_alloc( BLOCKSIZE, SIZE*SIZE*sizeof(TYPE) );
	TYPE* C = (TYPE*)aligned_alloc( BLOCKSIZE, SIZE*SIZE*sizeof(TYPE) );
	for( unsigned i = 0; i < SIZE; i++ ) {
		for( unsigned j = 0; j < SIZE; j++ ) {
			A[i*SIZE+j] = (double)i;
			B[i*SIZE+j] = (double)j;
			C[i*SIZE+j] = (TYPE)0.0;
		}
	}
	struct timespec start, end;
	while( clock_gettime(CLOCK_MONOTONIC, &start) ) {}
	#pragma omp parallel for
	for( int i = 0; i < SIZE; i += BLOCKSIZE )
	{
		for( int j = 0; j < SIZE; j += BLOCKSIZE )
		{
			for( int k = 0; k < SIZE; k += BLOCKSIZE )
			{
				do_block( SIZE, i, j, k, (TYPE*)A, (TYPE*)B, (TYPE*)C );
			}
		}
	}
	while( clock_gettime(CLOCK_MONOTONIC, &end) ) {}
	double time_s    = (double)end.tv_sec - (double)start.tv_sec;
	double time_ns   = ((double)end.tv_nsec - (double)start.tv_nsec) * pow( 10.0, -9.0 );
	double totalTime = time_s + time_ns;
    printf("Time: %fs\n", totalTime);

	// check our answer
	TYPE* D = (TYPE*)aligned_alloc( BLOCKSIZE, SIZE*SIZE*sizeof(TYPE) );
	for( unsigned i = 0; i < SIZE; i++ ) {
		for( unsigned j = 0; j < SIZE; j++ ) {
			A[i*SIZE+j] = (double)i;
			B[i*SIZE+j] = (double)j;
			D[i*SIZE+j] = (TYPE)0.0;
		}
	}
	for( unsigned i = 0; i < SIZE; i++ ) {
		for( unsigned j = 0; j < SIZE; j++ ) {
			for( unsigned k = 0; k < SIZE; k++ ) {
				D[i*SIZE+j] += A[i*SIZE+j] * B[i*SIZE+j];
			}
		}
	}
	double error = 0.0;
	double total = 0.0;
	for( unsigned i = 0; i < SIZE; i++ ) {
		for( unsigned j = 0; j < SIZE; j++ ) {
			for( unsigned k = 0; k < SIZE; k++ ) {
				error += fabs((double)(C[i*SIZE+j]-D[i*SIZE+j]));
				total += fabs((double)D[i*SIZE+j]);
			}
		}
	}
	printf("\nTotal error measured in the fast GEMM answer was %g%%\n", ((error-total) / total) * 100.0);
	
	return 0;
}
