
#include <math.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>

struct timespec __TIMINGLIB_START;
struct timespec __TIMINGLIB_END;

inline void __TIMINGLIB_start_time()
{
	clock_gettime(CLOCK_MONOTONIC, &__TIMINGLIB_START);
}

inline double __TIMINGLIB_end_time()
{
	clock_gettime(CLOCK_MONOTONIC, &__TIMINGLIB_END);
	double time_s  = (double)__TIMINGLIB_END.tv_sec - (double)__TIMINGLIB_START.tv_sec;
	double time_ns = ((double)__TIMINGLIB_END.tv_nsec - (double)__TIMINGLIB_START.tv_nsec) * pow(10.0, -9.0);
	printf("\n Running time: %gs\n", (time_s + time_ns));
	return time_s + time_ns;
}
