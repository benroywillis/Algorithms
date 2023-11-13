#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include "TimingLib.h"
#include "GEMV_autoschedule_false_generated.h"

#include "HalideBuffer.h"

int main(int argc, char **argv) {
	if( argc != 4 )
	{
		printf("Usage: M, N, num_threads\n");
		return 1;
	}
	int N = atoi(argv[1]);
	int M = atoi(argv[2]);
	int threads = atoi(argv[3]);
	printf("Setting thread count to %d\n", threads);
	halide_set_num_threads(threads);

	Halide::Runtime::Buffer<float> mat_a(nullptr, N, M);
	Halide::Runtime::Buffer<float> mat_b(nullptr, M);
	Halide::Runtime::Buffer<float> mat_c(nullptr, N);
	mat_a.allocate();
	mat_b.allocate();
	mat_c.allocate();

    double time = __TIMINGLIB_benchmark([&]() {
        auto out = GEMV_autoschedule_false_generated(mat_a, mat_b, mat_c);
    });
    printf("Success!\n");
    return 0;
}
