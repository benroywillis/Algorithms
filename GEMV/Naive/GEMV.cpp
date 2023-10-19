
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "TimingLib.h"

#ifndef TRACING
#define TRACING 0
#endif

#define PRECISION 	float
#define SIZE 		512
//#define SIZE 		64

void GEMM(PRECISION (*in0)[SIZE], PRECISION (*in1), PRECISION (*out))
{
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            out[i] += in0[i][j] * in1[j];
        }
    }
}

int main()
{
    PRECISION (*in0)[SIZE] = (PRECISION (*)[SIZE])malloc(sizeof(PRECISION[SIZE][SIZE]));
    PRECISION (*in1) = (PRECISION (*))malloc(sizeof(PRECISION[SIZE]));
    PRECISION (*out) = (PRECISION (*))malloc(sizeof(PRECISION[SIZE]));

    for (int i = 0; i < SIZE; i++)
    {
        in1[i] = rand();
        out[i] = 0;
        for (int j = 0; j < SIZE; j++)
        {
            in0[i][j] = rand();
        }
    }

	__TIMINGLIB_benchmark([&]{ GEMM(in0, in1, out); });

    return 0;
}
