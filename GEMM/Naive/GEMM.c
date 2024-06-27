
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#ifndef PRECISION
#define TYPE float
#elif PRECISION == 0
#define TYPE float
#elif PRECISION == 1
#define TYPE double
#endif
#ifndef SIZE
#define SIZE 		64
#endif

struct timespec __TIMINGLIB_START;
struct timespec __TIMINGLIB_END;
double __TIMINGLIB_array[TIMINGLIB_ITERATIONS];
uint8_t __TIMINGLIB_iterations = 0;

//void GEMM(TYPE (*in0)[SIZE], TYPE (*in1)[SIZE], TYPE (*out)[SIZE])
void GEMM(TYPE *in0, TYPE *in1, TYPE *out)
{
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            for (int k = 0; k < SIZE; k++)
            {
               // out[i][j] += in0[i][k] * in1[k][j];
               out[i*SIZE+j] += in0[i*SIZE+k] * in1[k*SIZE+j];
            }
        }
    }
}

void GEMM_polygeist(TYPE* in0, TYPE* in1, TYPE* out)
{
    double best = 1000000000.0;
    for (uint64_t i = 0; i < TIMINGLIB_SAMPLES; i++) {
        clock_gettime(CLOCK_MONOTONIC, &__TIMINGLIB_START);
        for (uint64_t j = 0; j < TIMINGLIB_ITERATIONS; j++) {
#pragma scop
 		    for (int i = 0; i < SIZE; i++)
    		{
        		for (int j = 0; j < SIZE; j++)
        		{
            		for (int k = 0; k < SIZE; k++)
            		{
                		//out[i][j] += in0[i][k] * in1[k][j];
                		out[i*SIZE+j] += in0[i*SIZE+k] * in1[k*SIZE+j];
            		}
        		}
    		}
#pragma endscop
        }
		clock_gettime(CLOCK_MONOTONIC, &__TIMINGLIB_END);
		double time_s  = (double)__TIMINGLIB_END.tv_sec - (double)__TIMINGLIB_START.tv_sec;
		double time_ns = ((double)__TIMINGLIB_END.tv_nsec - (double)__TIMINGLIB_START.tv_nsec) * pow(10.0, -9.0);
        double elapsed_seconds = time_s + time_ns;
		__TIMINGLIB_array[__TIMINGLIB_iterations] = elapsed_seconds;
        best = best > elapsed_seconds ? elapsed_seconds : best;
		__TIMINGLIB_iterations++;
    }
    printf("Average running time: %gs\n", best / TIMINGLIB_ITERATIONS);
}

int main()
{
    /*TYPE (*in0)[SIZE] = (TYPE (*)[SIZE])malloc(SIZE * sizeof(TYPE[SIZE][SIZE]));
    TYPE (*in1)[SIZE] = (TYPE (*)[SIZE])malloc(SIZE * sizeof(TYPE[SIZE][SIZE]));
    TYPE (*out)[SIZE] = (TYPE (*)[SIZE])malloc(SIZE * sizeof(TYPE[SIZE][SIZE]));*/
	TYPE* in0 = (TYPE*)malloc(SIZE*SIZE*sizeof(TYPE));
	TYPE* in1 = (TYPE*)malloc(SIZE*SIZE*sizeof(TYPE));
	TYPE* out = (TYPE*)malloc(SIZE*SIZE*sizeof(TYPE));
    //TYPE in0[SIZE][SIZE];
    //TYPE in1[SIZE][SIZE];
    //TYPE out[SIZE][SIZE];
    //TYPE in0[SIZE*SIZE];
    //TYPE in1[SIZE*SIZE];
    //TYPE out[SIZE*SIZE];

    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            //in0[i][j] = rand();
            //in1[i][j] = rand();
            //out[i][j] = 0;
            in0[i*SIZE+j] = rand();
            in1[i*SIZE+j] = rand();
            out[i*SIZE+j] = 0;
        }
    }

	clock_gettime(CLOCK_MONOTONIC, &__TIMINGLIB_START);
	GEMM(in0, in1, out);
	clock_gettime(CLOCK_MONOTONIC, &__TIMINGLIB_END);
	double time_s  = (double)__TIMINGLIB_END.tv_sec - (double)__TIMINGLIB_START.tv_sec;
	double time_ns = ((double)__TIMINGLIB_END.tv_nsec - (double)__TIMINGLIB_START.tv_nsec) * pow(10.0, -9.0);
    double elapsed_seconds = time_s + time_ns;
	printf("Time: %gs\n", elapsed_seconds);
	
	///GEMM_polygeist(in0, in1, out);

    return 0;
}
