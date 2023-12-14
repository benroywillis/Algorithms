
#include <stdio.h>
#include <stdlib.h>
#include "TimingLib.h"

#if PRECISION == 0 // float
#define TYPE float
#else // double
#define TYPE double
#endif

#ifndef SIZE
#define SIZE 		64
#endif

#ifndef THREADS_PER_BLOCK
#define THREADS_PER_BLOCK 256
#endif

#ifndef CHECK
#define CHECK 0
#endif

__global__
void GEMM(TYPE *in0, TYPE *in1, TYPE *out)
{
	int stride = blockDim.x * gridDim.x;
	int index  = blockIdx.x * blockDim.x + threadIdx.x;
    for (int i = index; i < SIZE; i += stride )
    {
        for (int j = 0; j < SIZE; j++)
        {
            for (int k = 0; k < SIZE; k++)
            {
                out[i*SIZE+j] += in0[i*SIZE+k] * in1[k*SIZE+j];
            }
        }
    }
}

int main()
{
	TYPE* in0, *in1, *out, *out_check;
	cudaMallocManaged(&in0, SIZE*SIZE*sizeof(TYPE));
	cudaMallocManaged(&in1, SIZE*SIZE*sizeof(TYPE));
	cudaMallocManaged(&out, SIZE*SIZE*sizeof(TYPE));
	cudaMallocManaged(&out_check, SIZE*SIZE*sizeof(TYPE));

    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            in0[i*SIZE+j] = rand();
            in1[i*SIZE+j] = rand();
            out[i*SIZE+j] = 0;
            out_check[i*SIZE+j] = 0;
        }
    }

	__TIMINGLIB_benchmark([&]{ GEMM<<< (SIZE + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK, THREADS_PER_BLOCK >>>(in0, in1, out); cudaDeviceSynchronize(); });

	// keeps the optimizer from ruining the experiment
	volatile bool yes = out[0];
	/*for( unsigned i = 0; i < SIZE; i++ )
	{
		for( unsigned j = 0; j < SIZE; j++ )
		{
			volatile bool yes = out[i*SIZE+j];
		}
	}*/
#if CHECK == 1
	print("Running check between CUDA answer and naive C answer...\n");
	for( unsigned i = 0; i < SIZE; i++ )
	{
		for( unsigned j = 0; j < SIZE; j++ )
		{
			for( unsigned k = 0; k < SIZE ; k++ )
			{
				out_check[i*SIZE+j] += in0[i*SIZE+k] * in1[k*SIZE+j];
			}
		}
	}
	double error = 0.0;
	double sum   = 0.0;
	for( unsigned i = 0; i < SIZE; i++ )
	{
		for( unsigned j = 0; j < SIZE; j++ )
		{
			error += abs(out[i*SIZE+j]  - out_check[i*SIZE+j]);
			sum   += abs(out[i*SIZE+j]) + abs(out_check[i*SIZE+j]);
		}
	}
	printf("Difference: %g\n", (error / sum) * 100);
#endif

	cudaFree(in0);
	cudaFree(in1);
	cudaFree(out);
	cudaFree(out_check);
    return 0;
}
