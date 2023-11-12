
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "TimingLib.h"

#define PRECISION 	float
#define SIZE 		512
//#define SIZE 		64

volatile PRECISION _innerProduct(PRECISION* in0, PRECISION* in1)
{
	volatile PRECISION out = (PRECISION)0.0;
    for (int i = 0; i < SIZE; i++)
    {
        out += in0[i] * in1[i];
    }
	return out;
}

int main()
{
    PRECISION* in0 = (PRECISION*)malloc(SIZE*sizeof(PRECISION));
    PRECISION* in1 = (PRECISION*)malloc(SIZE*sizeof(PRECISION));
    for (int i = 0; i < SIZE; i++)
    {
        in0[i] = rand();
        in1[i] = rand();
    }
	__TIMINGLIB_benchmark([&]{ _innerProduct(in0, in1); });
    return 0;
}
