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
#define THREADS_PER_BLOCK 256
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
        }                                                                 \
    } while (0)

__global__ void GEMM(TYPE *A, TYPE *B, TYPE *C)
{
	int stride = blockDim.x * gridDim.x;
	int index  = blockIdx.x * blockDim.x + threadIdx.x;
    for (int i = index; i < SIZE; i += stride )
    {
        for (int j = 0; j < SIZE; j++)
        {
            for (int k = 0; k < SIZE; k++)
            {
            	C[i*SIZE+j]+= A[i*SIZE+k] * B[k*SIZE+j];
            }
        }
    }
}

int main()
{
	TYPE* A = (TYPE*)malloc(sizeof(TYPE)*SIZE*SIZE);
	TYPE* B = (TYPE*)malloc(sizeof(TYPE)*SIZE*SIZE);
	TYPE* C = (TYPE*)malloc(sizeof(TYPE)*SIZE*SIZE);
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            A[i*SIZE+j] = fmod(rand(), SIZE);
            B[i*SIZE+j] = fmod(rand(), SIZE);
            C[i*SIZE+j] = 0;
        }
    }

    // create a stream
    cudaStream_t stream = NULL;
    CUDA_CHECK(cudaStreamCreateWithFlags(&stream, cudaStreamNonBlocking));
    // copy data to device
    TYPE* d_A;
    TYPE* d_B;
    TYPE* d_C;
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void **>(&d_A), sizeof(TYPE) * SIZE*SIZE));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void **>(&d_B), sizeof(TYPE) * SIZE*SIZE));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void **>(&d_C), sizeof(TYPE) * SIZE*SIZE));
    CUDA_CHECK(cudaMemcpyAsync((void*)d_A, A, sizeof(TYPE) * SIZE*SIZE, cudaMemcpyHostToDevice, stream));
    CUDA_CHECK(cudaMemcpyAsync((void*)d_B, B, sizeof(TYPE) * SIZE*SIZE, cudaMemcpyHostToDevice, stream));
	// run GEMM
	__TIMINGLIB_benchmark( [&] {
		GEMM<<< (SIZE + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK, THREADS_PER_BLOCK >>>(d_A, d_B, d_C); 
    	CUDA_CHECK(cudaStreamSynchronize(stream));
		CUDA_CHECK(cudaDeviceSynchronize());
	});
    // copy data to host
    CUDA_CHECK(cudaMemcpyAsync(C, d_C, sizeof(TYPE) * SIZE*SIZE, cudaMemcpyDeviceToHost, stream));

#if CHECK
	TYPE* D = (TYPE*)calloc(sizeof(TYPE), SIZE*SIZE);
	for( unsigned i = 0; i < SIZE; i++ )
	{
		for( unsigned j = 0; j < SIZE; j++ )
		{
			//printf("%g\n", C[i*SIZE+j]);
			for( unsigned k = 0; k < SIZE ; k++ )
			{
				D[i*SIZE+j] += A[i*SIZE+k] * B[k*SIZE+j];
			}
		}
	}
	__TIMINGLIB_snr(D, C, sizeof(TYPE), SIZE*SIZE);
	free(D);
#endif

	free(A);
	free(B);
	free(C);
    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);
    cudaStreamDestroy(stream);
    cudaDeviceReset();
    return 0;
}
