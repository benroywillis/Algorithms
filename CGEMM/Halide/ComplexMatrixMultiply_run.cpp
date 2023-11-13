#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <complex.h>

#include "TimingLib.h"
#include "ComplexMatrixMultiply_autoschedule_false_generated.h"

#include "HalideBuffer.h"

// 0 for float, 1 for double
#ifndef PRECISION
#define PRECISION 0
#endif

#if PRECISION == 0
#define TYPE float
#else
#define TYPE double
#endif

#ifndef SIZE
#define SIZE 512
#endif

int main(int argc, char **argv) {
	if( argc != 2 )
	{
		printf("Usage: num_threads\n");
		return 1;
	}
	int threads = atoi(argv[1]);
	printf("Setting thread count to %d\n", threads);
	halide_set_num_threads(threads);

	Halide::Runtime::Buffer<TYPE> mat_a(nullptr, SIZE, SIZE, 2);
	Halide::Runtime::Buffer<TYPE> mat_b(nullptr, SIZE, SIZE, 2);
	Halide::Runtime::Buffer<TYPE> mat_c(nullptr, SIZE, SIZE, 2);
	mat_a.allocate();
	mat_b.allocate();
	mat_c.allocate();

    double time = __TIMINGLIB_benchmark([&]() {
        auto out = ComplexMatrixMultiply_autoschedule_false_generated(mat_a, mat_b, mat_c);
    });
    printf("Success!\n");
    return 0;
}
