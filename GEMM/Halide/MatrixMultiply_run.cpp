#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include "TimingLib.h"
#if HALIDE_AUTOSCHEDULE == 1
#include "MatrixMultiply_autoschedule_true_generated.h"
#endif
#include "MatrixMultiply_autoschedule_false_generated.h"

#include "HalideBuffer.h"

#ifndef SIZE
#define SIZE 512
#endif

int main(int argc, char **argv) {
	if( argc != 4 )
	{
		printf("Usage: M, N, num_threads\n");
		return 1;
	}
	int N = atoi(argv[1]);
	int M = atoi(argv[2]);
	N = M = SIZE;
	int threads = atoi(argv[3]);
	printf("Setting thread count to %d\n", threads);
	halide_set_num_threads(threads);

	Halide::Runtime::Buffer<float> mat_a(nullptr, N, M);
	Halide::Runtime::Buffer<float> mat_b(nullptr, M, N);
	Halide::Runtime::Buffer<float> mat_c(nullptr, N, N);
	mat_a.allocate();
	mat_b.allocate();
	mat_c.allocate();

#if HALIDE_AUTOSCHEDULE == 1
	double autotime = __TIMINGLIB_benchmark([&]() {
		auto out = MatrixMultiply_autoschedule_true_generated(mat_a, mat_b, mat_c);
		mat_c.device_sync();
		mat_c.copy_to_host();
	});
#endif
    double time = __TIMINGLIB_benchmark([&]() {
        auto out = MatrixMultiply_autoschedule_false_generated(mat_a, mat_b, mat_c);
		mat_c.device_sync();
		mat_c.copy_to_host();
    });
    printf("Success!\n");
    return 0;
}
