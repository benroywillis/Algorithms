#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include "TimingLib.h"
#include "ElementMultiply_autoschedule_false_generated.h"

#include "HalideBuffer.h"

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

	Halide::Runtime::Buffer<float> mat_a(nullptr, SIZE, SIZE);
	Halide::Runtime::Buffer<float> mat_b(nullptr, SIZE, SIZE);
	Halide::Runtime::Buffer<float> mat_c(nullptr, SIZE, SIZE);
	mat_a.allocate();
	mat_b.allocate();
	mat_c.allocate();

    double time = __TIMINGLIB_benchmark([&]() {
        auto out = ElementMultiply_autoschedule_false_generated(mat_a, mat_b, mat_c);
    });
    printf("Success!\n");
    return 0;
}
