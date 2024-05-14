
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "TimingLib.h"


#ifndef PRECISION
#define TYPE float
#elif   PRECISION == 0
#define TYPE float
#elif   PRECISION == 1
#define TYPE double
#endif

#ifndef SIZE
#define SIZE 		64
#endif

//void GEMM(TYPE (*in0)[SIZE], TYPE (*in1)[SIZE], TYPE (*out)[SIZE])
void GEMM(TYPE *in0, TYPE *in1, TYPE *out)
{
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

void GEMM_polygeist(TYPE* in0, TYPE* in1, TYPE* out)
{
    double best = 1000000000.0;
    for (uint64_t i = 0; i < TIMINGLIB_SAMPLES; i++) {
        __TIMINGLIB_start_time();
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
        double elapsed_seconds = __TIMINGLIB_end_time();
        best = best > elapsed_seconds ? elapsed_seconds : best;
    }
    printf("Average running time: %gs\n", best / TIMINGLIB_ITERATIONS);
}

int main()
{
    /*TYPE (*in0)[SIZE] = (TYPE (*)[SIZE])malloc(sizeof(TYPE[SIZE][SIZE]));
    TYPE (*in1)[SIZE] = (TYPE (*)[SIZE])malloc(sizeof(TYPE[SIZE][SIZE]));
    TYPE (*out)[SIZE] = (TYPE (*)[SIZE])malloc(sizeof(TYPE[SIZE][SIZE]));
	*/
    TYPE *in0 = (TYPE *)malloc(sizeof(TYPE[SIZE][SIZE]));
    TYPE *in1 = (TYPE *)malloc(sizeof(TYPE[SIZE][SIZE]));
    TYPE *out = (TYPE *)malloc(sizeof(TYPE[SIZE][SIZE]));

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

	__TIMINGLIB_benchmark([&]{ GEMM(in0, in1, out); });
	//GEMM_polygeist(in0, in1, out);

	// to make the optimizer preserve the GEMM
	volatile bool is = out[0];

    return 0;
}
