
#include "Trace.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#ifndef TRACING
#define TRACING 0
#endif

#define SIZE 	512
//const int SIZE = 512;

int main()
{
    int(*in0)[SIZE] = malloc(SIZE * sizeof(int[SIZE][SIZE]));
    int(*in1)[SIZE] = malloc(SIZE * sizeof(int[SIZE][SIZE]));
    int(*out)[SIZE] = malloc(SIZE * sizeof(int[SIZE][SIZE]));

    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            in0[i][j] = rand();
            in1[i][j] = rand();
            out[i][j] = 0;
        }
    }

#if TRACING
   TraceAtlasMarkovKernelEnter("MatrixMultiply,Outer");
#endif
	struct timespec start, end;
	while( clock_gettime(CLOCK_MONOTONIC, &start) ) {}
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
	while( clock_gettime(CLOCK_MONOTONIC, &end) ) {}

	double time_s  = (double)end.tv_sec - (double)start.tv_sec;
	double time_ns = ((double)end.tv_nsec - (double)start.tv_nsec) * pow( 10.0, -9.0 );
	double total   = time_s + time_ns;
    printf("Time: %fs\n", total);
    return 0;
}
