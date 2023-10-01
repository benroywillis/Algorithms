#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include "TimingLib.h"
#include "MatrixMultiply_autoschedule_false_generated.h"

#include "HalideBuffer.h"

#ifndef HALIDE_THREADS
#define HALIDE_THREADS 1
#endif

int main(int argc, char **argv) {
	printf("Setting threads to %d\n", HALIDE_THREADS);
    halide_set_num_threads(HALIDE_THREADS);

	if( argc != 3 )
	{
		printf("Usage: M, N\n");
		return 1;
	}
	int N = atoi(argv[1]);
	int M = atoi(argv[2]);

	Halide::Runtime::Buffer<float> mat_a(nullptr, N, M);
	Halide::Runtime::Buffer<float> mat_b(nullptr, M, N);
	Halide::Runtime::Buffer<float> mat_c(nullptr, N, N);
	mat_a.allocate();
	mat_b.allocate();
	mat_c.allocate();

    double time = __TIMINGLIB_benchmark([&]() {
        auto out = MatrixMultiply_autoschedule_false_generated(mat_a, mat_b, mat_c);
    });
    printf("Success!\n");
    return 0;
}
