
#include <math.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <setjmp.h>
#include <functional>

#ifndef TIMINGLIB_SAMPLES
#define TIMINGLIB_SAMPLES 10
#endif

#ifndef TIMINGLIB_ITERATIONS
#define TIMINGLIB_ITERATIONS 10
#endif

struct timespec __TIMINGLIB_START;
struct timespec __TIMINGLIB_END;

double __TIMINGLIB_array[TIMINGLIB_ITERATIONS];
uint8_t __TIMINGLIB_iterations = 0;
jmp_buf __TIMINGLIB_buf;

void __TIMINGLIB_start_time()
{
	//setjmp(__TIMINGLIB_buf);
	clock_gettime(CLOCK_MONOTONIC, &__TIMINGLIB_START);
}

double __TIMINGLIB_end_time()
{
	clock_gettime(CLOCK_MONOTONIC, &__TIMINGLIB_END);
	double time_s  = (double)__TIMINGLIB_END.tv_sec - (double)__TIMINGLIB_START.tv_sec;
	double time_ns = ((double)__TIMINGLIB_END.tv_nsec - (double)__TIMINGLIB_START.tv_nsec) * pow(10.0, -9.0);
	//printf("\n Running time: %gs\n", (time_s + time_ns));
	__TIMINGLIB_array[__TIMINGLIB_iterations] = time_s + time_ns;
	__TIMINGLIB_iterations++;
	/*if( __TIMINGLIB_iterations < __TIMINGLIB_ITERATIONS )
	{
		longjmp(__TIMINGLIB_buf, 2000+__TIMINGLIB_iterations);
	}*/
	return time_s + time_ns;
}

inline double __TIMINGLIB_benchmark(uint64_t samples, uint64_t iterations, const std::function<void()> &op) {
    double best = 1000000000.0;
    for (uint64_t i = 0; i < samples; i++) {
        __TIMINGLIB_start_time();
        for (uint64_t j = 0; j < iterations; j++) {
            op();
        }
        double elapsed_seconds = __TIMINGLIB_end_time();
        best = best > elapsed_seconds ? elapsed_seconds : best;
    }
	printf("Average running time: %gs\n", best / iterations);
    return best / iterations;
}

inline double __TIMINGLIB_benchmark(const std::function<void()> &op) {
    double best = 1000000000.0;
    for (uint64_t i = 0; i < TIMINGLIB_SAMPLES; i++) {
        __TIMINGLIB_start_time();
        for (uint64_t j = 0; j < TIMINGLIB_ITERATIONS; j++) {
            op();
        }
        double elapsed_seconds = __TIMINGLIB_end_time();
        best = best > elapsed_seconds ? elapsed_seconds : best;
    }
	printf("Average running time: %gs\n", best / TIMINGLIB_ITERATIONS);
    return best / TIMINGLIB_ITERATIONS;
}


