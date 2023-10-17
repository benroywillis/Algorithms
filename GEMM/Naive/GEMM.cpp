
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "TimingLib.h"

#ifndef TRACING
#define TRACING 0
#endif

#define PRECISION 	float
//#define SIZE 		512
#define SIZE 		64

void GEMM(PRECISION (*in0)[SIZE], PRECISION (*in1)[SIZE], PRECISION (*out)[SIZE])
{
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            for (int k = 0; k < SIZE; k++)
            {
                out[i][j] += in0[i][k] * in1[k][j];
            }
        }
    }
}

int main()
{
    PRECISION (*in0)[SIZE] = (PRECISION (*)[SIZE])malloc(sizeof(PRECISION[SIZE][SIZE]));
    PRECISION (*in1)[SIZE] = (PRECISION (*)[SIZE])malloc(sizeof(PRECISION[SIZE][SIZE]));
    PRECISION (*out)[SIZE] = (PRECISION (*)[SIZE])malloc(sizeof(PRECISION[SIZE][SIZE]));

    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            in0[i][j] = rand();
            in1[i][j] = rand();
            out[i][j] = 0;
        }
    }

	__TIMINGLIB_benchmark([&]{ GEMM(in0, in1, out); });

    return 0;
}
