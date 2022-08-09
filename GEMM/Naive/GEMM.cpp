
#include "Trace.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "TimingLib.h"

#ifndef TRACING
#define TRACING 0
#endif

#define SIZE 	512
//const int SIZE = 512;

void GEMM(int (*in0)[SIZE], int (*in1)[SIZE], int (*out)[SIZE])
{
#if TRACING
   TraceAtlasMarkovKernelEnter("MatrixMultiply,Outer");
#endif
    for (int i = 0; i < SIZE; i++)
    {
#if TRACING
        TraceAtlasMarkovKernelEnter("MatrixMultiply,Inner");
#endif
        for (int j = 0; j < SIZE; j++)
        {
#if TRACING
            TraceAtlasMarkovKernelEnter("MatrixMultiply,Mul");
#endif
            for (int k = 0; k < SIZE; k++)
            {
                out[i][j] += in0[i][k] * in1[k][j];
            }
#if TRACING
            TraceAtlasMarkovKernelExit("MatrixMultiply,Mul");
#endif
        }
#if TRACING
        TraceAtlasMarkovKernelExit("MatrixMultiply,Inner");
#endif
    }
#if TRACING
    TraceAtlasMarkovKernelExit("MatrixMultiply,Outer");
#endif
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

	__TIMINGLIB_benchmark(10, 10, [&]{ GEMM(in0, in1, out); });

    return 0;
}
