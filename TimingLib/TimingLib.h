
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

#ifndef PRINT_TIMES
#define PRINT_TIMES 0
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

double __TIMINGLIB_benchmark(uint64_t samples, uint64_t iterations, const std::function<void()> &op) {
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

double __TIMINGLIB_benchmark(const std::function<void()> &op) {
	double times[TIMINGLIB_SAMPLES];
    for (uint64_t i = 0; i < TIMINGLIB_SAMPLES; i++) {
        __TIMINGLIB_start_time();
        for (uint64_t j = 0; j < TIMINGLIB_ITERATIONS; j++) {
            op();
        }
        double elapsed_seconds = __TIMINGLIB_end_time();
		//printf("%g\n", elapsed_seconds);
		times[i] = elapsed_seconds/TIMINGLIB_ITERATIONS;
    }
	for( int i = 0; i < TIMINGLIB_SAMPLES; i++ ) {
		for( int j = i+1; j < TIMINGLIB_SAMPLES; j++ ) {
			if( times[i] > times[j] ) {
				double more = times[i];
				times[i] = times[j];
				times[j] = more;
			}
		}
	}
#if PRINT_TIMES
	for( int i = 0; i < TIMINGLIB_SAMPLES; i++ ) {
		printf("%g\n", times[i]);
	}
#endif
	double median = 0.0;
	if( (TIMINGLIB_SAMPLES % 2) && (TIMINGLIB_SAMPLES > 1) ) {
		median = (times[TIMINGLIB_SAMPLES/2] + times[TIMINGLIB_SAMPLES/2+1])/2;
	}
	else {
		median = times[TIMINGLIB_SAMPLES/2];
	}
	printf("Median running time: %gs\n", median);
    return median;
}

inline void __TIMINGLIB_snr( void* ref, void* test, int elem_size, int num_elems )
{
    double num = 0.0;
    double den = 0.0;
	if( elem_size == 1 ) {
		for( unsigned i = 0; i < num_elems; i++ ) {
			// num += ref*ref
        	num += (double)((uint8_t*)ref)[i] * (double)((uint8_t*)ref)[i];
			// den += (ref-test)*(ref-test)
        	den += (double)( ((uint8_t*)ref)[i] - ((uint8_t*)test)[i] ) * (double)( ((uint8_t*)ref)[i] - ((uint8_t*)test)[i] );
    	}
	}
	else if( elem_size == 4 ) {
		for( unsigned i = 0; i < num_elems; i++ ) {
			// num += ref*ref
        	num += (double)((float*)ref)[i] * (double)((float*)ref)[i];
			// den += (ref-test)*(ref-test)
        	den += (double)( ((float*)ref)[i] - ((float*)test)[i] ) * (double)( ((float*)ref)[i] - ((float*)test)[i] );
    	}
	}
	else if( elem_size == 8 ) {
		for( unsigned i = 0; i < num_elems; i++ ) {
			// num += ref*ref
        	num +=  ((double*)ref)[i] * ((double*)ref)[i];
			// den += (ref-test)*(ref-test)
        	den += ( ((double*)ref)[i] - ((double*)test)[i] ) * ( ((double*)ref)[i] - ((double*)test)[i] );
    	}
	}
	else { 
		printf("__TIMINGLIB_snr cannot handle data types that are not 1, 4 or 8 bytes large!\n"); 
		return; 
	}
    if (den < 0.001) printf("Reference and test outputs matched exactly\n");
    else             printf("num: %g den: %g\n", num, den); printf("SNR: %.2fdb\n", 10.0*log10(num/den));
}
