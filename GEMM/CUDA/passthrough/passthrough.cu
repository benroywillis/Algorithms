// helpful resource: https://timdettmers.com/2023/01/30/which-gpu-for-deep-learning/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
#define THREADS_PER_BLOCK 1
#endif

#ifndef CHECK
#define CHECK 0
#endif

// CUDA API error checking
#define CUDA_CHECK(err)                                                   \
    do {                                                                  \
        int err_ = (err);                                                 \
        if (err_ != 0) {                                                  \
            printf("CUDA error %d at %s:%d\n", err_, __FILE__, __LINE__); \
            return 1;                                                     \
        }                                                                 \
    } while (0)

__global__ 
void passthrough(TYPE *kernelA, TYPE *kernelB)
{
	//int stride = blockDim.x * gridDim.x;
	//int index  = blockIdx.x * blockDim.x + threadIdx.x;
    //for (int i = index; i < SIZE; i += stride )
	for( int i = 0; i < SIZE*SIZE; i++ ) {
		kernelB[i] = kernelA[i];
		//for( int j = 0; j < SIZE; j++ ) {
			//kernelB[i*SIZE+j] = kernelA[i*SIZE+j];
		//}
	}
}

int main()
{
	TYPE* A = (TYPE*)malloc(sizeof(TYPE)*SIZE*SIZE);
	TYPE* B = (TYPE*)malloc(sizeof(TYPE)*SIZE*SIZE);
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            A[i*SIZE+j] = fmod(rand(), SIZE);
            B[i*SIZE+j] = 6;
        }
    }

    // copy data to device
    TYPE* d_A;
    TYPE* d_B;
    CUDA_CHECK(cudaMalloc((void **)&d_A, sizeof(TYPE) * SIZE*SIZE));
    CUDA_CHECK(cudaMalloc((void **)&d_B, sizeof(TYPE) * SIZE*SIZE));
    CUDA_CHECK(cudaMemcpy((void*)d_A, A, sizeof(TYPE) * SIZE*SIZE, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy((void*)d_B, B, sizeof(TYPE) * SIZE*SIZE, cudaMemcpyHostToDevice));
	// run passthrough
	passthrough<<< 1, 1 >>>(d_A, d_B); 
    // copy data to host
	CUDA_CHECK(cudaDeviceSynchronize());
    CUDA_CHECK(cudaMemcpy(B, d_B, sizeof(TYPE) * SIZE*SIZE, cudaMemcpyDeviceToHost));

#if CHECK
	for( unsigned i = 0; i < SIZE; i++ )
	{
		for( unsigned j = 0; j < SIZE; j++ )
		{
			//printf("%g\n", A[i*SIZE+j]);
			printf("%g\n", B[i*SIZE+j]);
		}
	}
	__TIMINGLIB_snr(A, B, sizeof(TYPE), SIZE*SIZE);
#endif

	free(A);
	free(B);
    cudaFree(d_A);
    cudaFree(d_B);
    cudaDeviceReset();
    return 0;
}
