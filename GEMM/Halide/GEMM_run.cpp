#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
//#include "GEMM.h"
//#ifndef NO_AUTO_SCHEDULE
//#include "GEMM_autoschedule_true_generated.h"
#include "GEMM_autoschedule_false_generated.h"
//#endif

#include "HalideBuffer.h"
#include "halide_benchmark.h"
#include "halide_image_io.h"

#define SIZE 		512
#define PRECISION	double

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    int timing_iterations = 15;
	//PRECISION* A = (PRECISION*)malloc(SIZE*SIZE*sizeof(PRECISION));
	//PRECISION* B = (PRECISION*)malloc(SIZE*SIZE*sizeof(PRECISION));
	//PRECISION* C = (PRECISION*)calloc(SIZE*SIZE,sizeof(PRECISION));
	PRECISION A[SIZE][SIZE];
	PRECISION B[SIZE][SIZE];
	PRECISION C[SIZE][SIZE];
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
	halide_dimension_t arrayDims[] = {{0, SIZE, 1} , {0, SIZE, 1}};
    Buffer<double> input0( &A[0][0], 2, arrayDims );
    Buffer<double> input1( &B[0][0], 2, arrayDims );
    Buffer<double> output( &C[0][0], 2, arrayDims );

    // Manually-tuned version
    double min_t_manual = benchmark(timing_iterations, 10, [&]() {
        GEMM_autoschedule_false_generated(input0, input1, output);
        output.device_sync();
    });
    printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);
    return 0;
}
