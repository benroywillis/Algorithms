
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "TimingLib.h"

#ifndef TRACING
#define TRACING 0
#endif

#define SIZE 	64
//#define SIZE 	512
//const int SIZE = 512;

void GEMM(int (*in0)[SIZE], int (*in1)[SIZE], int (*out)[SIZE])
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
    int(*in0)[SIZE] = (int (*)[SIZE])malloc(SIZE * sizeof(int[SIZE][SIZE]));
    int(*in1)[SIZE] = (int (*)[SIZE])malloc(SIZE * sizeof(int[SIZE][SIZE]));
    int(*out)[SIZE] = (int (*)[SIZE])malloc(SIZE * sizeof(int[SIZE][SIZE]));

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
