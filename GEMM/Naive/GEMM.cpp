
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "TimingLib.h"

#define PRECISION 	float
#ifndef SIZE
#define SIZE 		64
#endif

//void GEMM(PRECISION (*in0)[SIZE], PRECISION (*in1)[SIZE], PRECISION (*out)[SIZE])
void GEMM(PRECISION *in0, PRECISION *in1, PRECISION *out)
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

void GEMM_polygeist(PRECISION* in0, PRECISION* in1, PRECISION* out)
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
    /*PRECISION (*in0)[SIZE] = (PRECISION (*)[SIZE])malloc(sizeof(PRECISION[SIZE][SIZE]));
    PRECISION (*in1)[SIZE] = (PRECISION (*)[SIZE])malloc(sizeof(PRECISION[SIZE][SIZE]));
    PRECISION (*out)[SIZE] = (PRECISION (*)[SIZE])malloc(sizeof(PRECISION[SIZE][SIZE]));
	*/
    PRECISION *in0 = (PRECISION *)malloc(sizeof(PRECISION[SIZE][SIZE]));
    PRECISION *in1 = (PRECISION *)malloc(sizeof(PRECISION[SIZE][SIZE]));
    PRECISION *out = (PRECISION *)malloc(sizeof(PRECISION[SIZE][SIZE]));

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
