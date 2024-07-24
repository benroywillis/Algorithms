// helpful resource: https://timdettmers.com/2023/01/30/which-gpu-for-deep-learning/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "TimingLib.h"

// some macros to make static configurations flexible
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

#define CEIL_DIV(M, N) (((M) + (N)-1) / (N))

#if PRECISION == 0 // float
#define TYPE float
#else // double
#define TYPE double
#endif

// input dimensions ( MxN = MxK * KxN )
#ifndef M
#define M	512
#endif
#ifndef K
#define K	512
#endif
#ifndef N
#define N 	512
#endif

// block dimensions
#ifndef BM
#define BM 	64
#endif
#ifndef BK
#define BK 	8
#endif 
#ifndef BN
#define BN 	64
#endif

// tile dimensions
#ifndef TM
#define TM	8
#endif
#ifndef TN
#define TN 	8
#endif

// to compare results on the GPU v CPU using SNR (see TimingLib.h for technicals)
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

// __launch_bounds__( maxThreadsPerblock, minBlocksPerSM, maxBlocksPerCluster) aids the compiler in register mapping
__global__ void __launch_bounds__((BM * BN) / (TM * TN), 1) GEMM(TYPE *A, TYPE *B, TYPE *C)
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

	// we flip rows and columns 
	unsigned int row = blockIdx.y;
	unsigned int col = blockIdx.x;

	// total number of output atoms (in C matrix) computed by a block
	const unsigned totalResultsBlockTile = BM*BN;
	// total number of output atoms calculated by each thread (in the block)
	const unsigned numThreadsBlockTile = totalResultsBlockTile / (TM*TN);
	assert( numThreadsBlockTile == blockDim.x );

	// allocate local buffers for current invocation in shared mem
	__shared__ TYPE As[BM*BK];
	__shared__ TYPE Bs[BK*BN];

	// we access individual threads in the block with these
	// remember the threadIdx's are projected onto the same number line (of size BM*BN) to allow for grouping contiguous column accesses together ("memory coalescing")
	// then the row index within the block will count every time BN entries has been complete (because their are BN entries in a row)
	// tiles (TM*TN) break up the block into sections, thus we normalize by the number of those sections to get the iterator range for a given block-tile permutation
	unsigned int threadRow = threadIdx.x / (BN/TN);
	// and the column index within the block counter will count every increment, and should stay within BN entries (because there are BN columns in each block)
	unsigned int threadCol = threadIdx.x % (BN/TN);

	// offset input/output working sets to the row/column squares that feed this kernel instance
	A += row * BM*K;
	B += col * BN;
	C += row * BM*N+ col*BN; 

	// calculate tile iterators
	// each thread tile is on a small square of the block 
	const int innerRowA = threadIdx.x / BK; // row counter increments each time we complete a row in the tile (with BK entries)
	const int innerColA = threadIdx.x % BK; // column counter increments each time and resets after completing a row (with BK entries)
	const unsigned strideA = numThreadsBlockTile / BK; // stride skips over each tile - remember all threads are on the same number line - thus after completing a tile in A, we move onto the next
	const int innerRowB = threadIdx.x / BN;
	const int innerColB = threadIdx.x % BN;
	const unsigned strideB = numThreadsBlockTile / BN; // stride skips over a completed tile - remember all threads are on the same number line - thus after completing a tile in B, we move to the next

	// these are partial sums calculated for each tile - at the end they are added to the applicable output pixel
	TYPE threadSums[TM*TN] = {(TYPE)0};
	// it is useful to put input values inside registers to save the redundant shared memory access (e.g., when adjacent tiles both need the same row, just register file that row)
	TYPE regM[TM] = { (TYPE)0 };
	TYPE regN[TN] = { (TYPE)0 };

	for( unsigned blkIdx = 0; blkIdx < K; blkIdx += BK ) { 
		// for each block (blkIdx), load A input working set into shared memory
		for( unsigned i = 0; i < BM; i += strideA ) {
			As[ (innerRowA + i) * BK + innerColA ] = A[ (innerRowA + i) * K + innerColA ];
		}
		for( unsigned i = 0; i < BK; i += strideB ) {
			Bs[ (innerRowB + i) * BN + innerColB ] = B[ (innerRowB + i) * N + innerColB ];
		}
		// here we have to block until all threads in the warp have initialized shared memory 
		__syncthreads();

		// now we update our base pointers to the next block
		A += BK;
		B += BK*N;

		// per-thread result (calculates a reduction over a single tile)
		for( unsigned dotIdx = 0; dotIdx < TM; dotIdx++ ) {
			// initialize block registers
			for( unsigned i = 0; i < TM; i++ ) {
				regM[i] = As[ (threadRow*TM + i)*BK + dotIdx ];
			}
			for( unsigned i = 0; i < TN; i++ ) {
				regN[i] = Bs[  dotIdx*BN + threadCol*TN + i];
			}
			for( unsigned threadTileA = 0; threadTileA < TM; threadTileA++ ) {
				for( unsigned threadTileB = 0; threadTileB < TN; threadTileB++ ) {
					threadSums[threadTileA*TN + threadTileB] += regM[ threadTileA ]*regN[ threadTileB ];
				}
			}
		}
		// again we block because all partial dot products need to be written before we move to another block
		__syncthreads();
	}
	// this is the accumulation of our block's sum into the result
	for( unsigned i = 0; i < TM; i++ ) {
		for( unsigned j = 0; j < TN; j++ ) {
			C[(threadRow*TM + i)*N + threadCol*TN + j] += threadSums[i*TM + j];
		}
	}
}

int main()
{
	TYPE* A = (TYPE*)malloc(sizeof(TYPE)*M*K);
	TYPE* B = (TYPE*)malloc(sizeof(TYPE)*K*N);
	TYPE* C = (TYPE*)malloc(sizeof(TYPE)*M*N);
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < K; j++) {
            A[i*K+j] = (TYPE)fmod(rand(), M);
        }
		for( int j = 0; j < N; j++ ) {
            C[i*N+j] = (TYPE)0;
		}
    }
	for( int i = 0; i < K; i++ ) {
		for( int j = 0; j < N; j++ ) {
            B[i*N+j] = (TYPE)fmod(rand(), N);
		}
	}

    // create a stream
    cudaStream_t stream = NULL;
    CUDA_CHECK(cudaStreamCreateWithFlags(&stream, cudaStreamNonBlocking));
    // copy data to device
    TYPE* d_A;
    TYPE* d_B;
    TYPE* d_C;
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void **>(&d_A), sizeof(TYPE) * M*K));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void **>(&d_B), sizeof(TYPE) * K*N));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void **>(&d_C), sizeof(TYPE) * M*N));
    CUDA_CHECK(cudaMemcpyAsync((void*)d_A, A, sizeof(TYPE) * M*K, cudaMemcpyHostToDevice, stream));
    CUDA_CHECK(cudaMemcpyAsync((void*)d_B, B, sizeof(TYPE) * K*N, cudaMemcpyHostToDevice, stream));
	// run GEMM
	__TIMINGLIB_benchmark( [&] {
		dim3 gridDim ( CEIL_DIV(N, BN), CEIL_DIV(M, BM) ); // the grid dim arranges blocks into groups
        dim3 blockDim( (BM * BN) / (TM*TN) ); // we make this one dimensional to enable memory coalescing - we use it to index both the row and column in a single dimension without doing weird math
		cudaFuncSetAttribute( GEMM, cudaFuncAttributePreferredSharedMemoryCarveout, cudaSharedmemCarveoutMaxShared );
		GEMM<<< gridDim, blockDim >>>(d_A, d_B, d_C); 
    	CUDA_CHECK(cudaStreamSynchronize(stream));
		CUDA_CHECK(cudaDeviceSynchronize());
	});
    // copy data to host
    CUDA_CHECK(cudaMemcpyAsync(C, d_C, sizeof(TYPE) * M*N, cudaMemcpyDeviceToHost, stream));

#if CHECK
	TYPE* D = (TYPE*)calloc(sizeof(TYPE), M*N);
	for( unsigned i = 0; i < M; i++ )
	{
		for( unsigned j = 0; j < N; j++ )
		{
			//printf("%g\n", C[i*SIZE+j]);
			for( unsigned k = 0; k < K; k++ )
			{
				D[i*N+j] += A[i*K+k] * B[k*N+j];
			}
		}
	}
	__TIMINGLIB_snr(D, C, sizeof(TYPE), M*N);
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
