#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <time.h>
#include <math.h>
#include "GEMM_config.h"
//#include "GEMM.h"
//#ifndef NO_AUTO_SCHEDULE
//#include "GEMM_autoschedule_true_generated.h"
#include "GEMM_autoschedule_false_generated.h"
//#endif

#include "HalideBuffer.h"
#include "halide_benchmark.h"
#include "halide_image_io.h"

#ifndef TOLERANCE
#define TOLERANCE 0.001
#endif

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
	PRECISION (*A)[SIZE] = (PRECISION (*)[SIZE])malloc( sizeof(PRECISION) * SIZE * SIZE);
	PRECISION (*B)[SIZE] = (PRECISION (*)[SIZE])malloc( sizeof(PRECISION) * SIZE * SIZE);
	PRECISION (*C)[SIZE] = (PRECISION (*)[SIZE])malloc( sizeof(PRECISION) * SIZE * SIZE);
	PRECISION (*test)[SIZE] = (PRECISION (*)[SIZE])malloc( sizeof(PRECISION) * SIZE * SIZE);
	for( unsigned int i = 0; i < SIZE; i++ )
	{
		for( unsigned int j = 0; j < SIZE; j++ )
		{
			//*(A + i*SIZE + j) = (PRECISION)rand();
			//*(B + i*SIZE + j) = (PRECISION)rand();
			A[i][j] = (PRECISION)rand();
			B[i][j] = (PRECISION)rand();
		}
	}
	// min index, max index, stride
    Buffer<PRECISION> input0( (PRECISION*)A, SIZE, SIZE );
    Buffer<PRECISION> input1( (PRECISION*)B, SIZE, SIZE );
    Buffer<PRECISION> output( (PRECISION*)C, SIZE, SIZE );
    //Buffer<PRECISION> output(SIZE, SIZE);
	//output.allocate();

    // Manually-tuned version
    int timing_iterations = 1;
    double min_t_manual = benchmark(timing_iterations, 10, [&]() {
        GEMM_autoschedule_false_generated(input0, input1, output);
        output.device_sync();
    });
    printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);
	// test matrix
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	for( int i = 0; i < SIZE; i++ )
	{
		for( int j = 0; j < SIZE; j++ )
		{
			for( int k = 0; k < SIZE; k++ )
			{
				test[i][j] += A[i][k]*B[k][j];
			}
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &end);
	double time_s = end.tv_sec - start.tv_sec;
	double time_ns = end.tv_nsec - start.tv_nsec;
	double totalTime = time_s + time_ns*pow( 10.0, -9.0 );
	printf("Naive time: %.2fms\n", totalTime*1000);

	int badMatch = 0;
	// compare the two results
	for( int i = 0; i < SIZE; i++ )
	{
		for( int j = 0; j < SIZE; j++ )
		{
			if( (A[i][j] != input0(j, i)) || (B[i][j] != input1(j, i)) )
			{
				badMatch++;
			}
		}
	}
	printf("%.2f%% of the inputs did not match!\n", (float)((float)badMatch / (float)(SIZE*SIZE) * 100 ) );
	badMatch = 0;
	// compare the two results
	for( int i = 0; i < SIZE; i++ )
	{
		for( int j = 0; j < SIZE; j++ )
		{
#if (PRECISION == double)
			if( fabs( (test[i][j] - output(j, i)) / output(j, i) ) > TOLERANCE )
			//if( fabs(test[i][j] - C[i][j]) > TOLERANCE )
			//if( fabs(test[i][j] - C[j][i]) > TOLERANCE )
#elif (PRECISION == float)
			if( fabsf( (test[i][j] - output(j, i)) / output(j, i) ) > TOLERANCE )
			//if( fabsf( (test[i][j] - C[j][i]) > TOLERANCE )
#else
			if( test[i][j] != output(j, i) )
			//if( test[i][j] != output(j, i) )
#endif
			{
				printf("%g, %g\n", test[i][j], output(j, i) );
				badMatch++;
			}
		}
	}
	printf("%.2f%% of the outputs did not match!\n", (float)((float)badMatch / (float)(SIZE*SIZE) * 100 ) );

    return 0;
}
