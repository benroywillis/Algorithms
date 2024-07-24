// helpful resource: https://timdettmers.com/2023/01/30/which-gpu-for-deep-learning/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "TimingLib.h"

// some macros to make static configurations flexible
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

#if PRECISION == 0 // float
#define TYPE float
#else // double
#define TYPE double
#endif

#ifndef SIZE
#define SIZE 		64
#endif

// defines how large the side of each block is in shared mem
#ifndef BLOCKSIZE
#define BLOCKSIZE 	32
#endif

#ifndef THREADS_PER_BLOCK
#define THREADS_PER_BLOCK 32
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
	// Streaming Multiprocessor (SM): holds 32 "warps" or "blocks"
    // block: a collection of warps that can be batched to an SM
    // warp: a collection of 32 threads
 	// thread: a specific invocation of the kernel definition
    // RTX3060 
    // - 32 warps/SM (64k 4-byte registers per SM)
    // - 32 threads/warp (64k 4-byte registes max per block)
    // - 255 4-byte registers per thread
    // - 
	// gridDim indexes into the grid (each index is a warp)
    // blockDim indexes into warp (each index is a thread)


	// thread IDs are useful for sharing memory among threads that all load the same column ("global memory coalescing")
	//uint32_t threadId = threadIdx.x + blockDim.x*(threadIdx.y+blockDim.y*threadIdx.z);

	// naive implementation
	//uint32_t x = blockIdx.x*blockDim.x + threadIdx.x;
	//uint32_t y = blockIdx.y*blockDim.y + threadIdx.y;
	
    // memory coalescing implementation
	// it shares the input data load ("memory coalescing") by telling each kernel invocation to access the same row, and touch different columns
	/*int x = blockIdx.x*32 + (threadIdx.x / 32);
	int y = blockIdx.y*32 + (threadIdx.x % 32);
    if ( x < SIZE && y < SIZE ) {
		TYPE tmp = (TYPE)0;
        for (int k = 0; k < SIZE; k++) {
            tmp += A[x*SIZE+k] * B[k*SIZE+y];
        }
		C[x*SIZE+y] = tmp;
    }*/

	// blocked and tiled version
	unsigned int row = blockIdx.x;
	unsigned int col = blockIdx.y;

	// allocate local buffers for current invocation in shared mem
	__shared__ TYPE As[min(SIZE, BLOCKSIZE*BLOCKSIZE)];
	__shared__ TYPE Bs[min(SIZE, BLOCKSIZE*BLOCKSIZE)];

	// we access individual entries in the block with these
	unsigned int threadRow = threadIdx.x / BLOCKSIZE;
	unsigned int threadCol = threadIdx.x % BLOCKSIZE;

	// offset our pointers to the correct block position
	A += row * BLOCKSIZE*SIZE; // replace SIZE with K if not square
	B += col * BLOCKSIZE;
	C += row * BLOCKSIZE*SIZE + col*BLOCKSIZE; // replace SIZE with N if not square

	// this sum holds the result of all block invocations
	float sum = (TYPE)0;
	for( unsigned blkIdx = 0; blkIdx < SIZE; blkIdx += BLOCKSIZE ) { 
		// load from RAM into shared memory
		As[threadRow*BLOCKSIZE+threadCol] = A[threadRow*SIZE + threadCol];
		Bs[threadRow*BLOCKSIZE+threadCol] = B[threadRow*SIZE + threadCol];

		// here we have to block until all threads in the warp have initialized shared memory 
		__syncthreads();
		
		// now we update our base pointers to the next block
		A += BLOCKSIZE;
		B += BLOCKSIZE*SIZE; // replace SIZE with N if the matrix is not square

		// now that shared memory is ready, we accumulate
		for( unsigned dotIdx = 0; dotIdx < BLOCKSIZE; dotIdx++ ) {
			sum += As[threadRow*BLOCKSIZE + dotIdx]*Bs[dotIdx*BLOCKSIZE+threadCol];
		}

		// again we block because all partial dot products need to be written before we move to another block
		__syncthreads();
	}
	// this is the accumulation of our block's sum into the result
	C[threadRow*SIZE + threadCol] += sum; // replace SIZE with N if matrix is not square
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
            A[i*SIZE+j] = (TYPE)fmod(rand(), SIZE);
            B[i*SIZE+j] = (TYPE)fmod(rand(), SIZE);
            C[i*SIZE+j] = (TYPE)0;
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
		dim3 gridDim ( BLOCKSIZE, BLOCKSIZE ); // the grid dim arranges blocks into groups
        // dim3 blockDim( BLOCKSIZE, BLOCKSIZE, 1 ); // this is the naive implementation
        dim3 blockDim( min( SIZE, BLOCKSIZE * BLOCKSIZE ) ); // we make this one dimensional to enable memory coalescing - we use it to index both the row and column in a single dimension without doing weird math
		cudaFuncSetAttribute( GEMM, cudaFuncAttributePreferredSharedMemoryCarveout, cudaSharedmemCarveoutMaxShared );
		GEMM<<< gridDim, blockDim >>>(d_A, d_B, d_C); 
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
