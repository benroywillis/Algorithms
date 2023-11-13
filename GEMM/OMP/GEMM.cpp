
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include "TimingLib.h"

#ifndef PRECISION
#define PRECISION 0
#endif
#if PRECISION == 0
#define TYPE float
#elif PRECISION == 1
#define TYPE float
#elif PRECISION == 2
#define TYPE int
#elif PRECISION == 3
#define TYPE long
#endif

#ifndef SIZE
#define SIZE 		512
#endif

void GEMM(TYPE (*in0)[SIZE], TYPE (*in1)[SIZE], TYPE (*out)[SIZE])
{
#pragma omp parallel for
    for (int i = 0; i < SIZE; i++)
    {
//#pragma omp parallel for
        for (int j = 0; j < SIZE; j++)
        {
			TYPE sum = (TYPE)0;
// this makes no significant difference compared to "pragma omp simd"
#pragma omp simd reduction(+:sum)
            for (int k = 0; k < SIZE; k++)
            {
                sum += in0[i][k] * in1[k][j];
            }
			out[i][j] = sum;
        }
    }
}

int main()
{
    TYPE (*in0)[SIZE] = (TYPE (*)[SIZE])malloc(sizeof(TYPE[SIZE][SIZE]));
    TYPE (*in1)[SIZE] = (TYPE (*)[SIZE])malloc(sizeof(TYPE[SIZE][SIZE]));
    TYPE (*out)[SIZE] = (TYPE (*)[SIZE])malloc(sizeof(TYPE[SIZE][SIZE]));

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
