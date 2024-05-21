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

// square matrix dimensions
#ifndef SIZE
#define SIZE 		64
#endif

#ifndef K
#define K 5 // KxK square filter
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

// discrete approximation for a 5x5 gaussian filter with mu = 0 and sigma = 1
__device__ TYPE filter_device[K][K] = { {(TYPE)1.0 / (TYPE)273.0,  (TYPE)4.0 / (TYPE)273.0,  (TYPE)7.0 / (TYPE)273.0,  (TYPE)4.0 / (TYPE)273.0, (TYPE)1.0 / (TYPE)272.0},
                            {(TYPE)4.0 / (TYPE)273.0, (TYPE)16.0 / (TYPE)273.0, (TYPE)26.0 / (TYPE)273.0, (TYPE)16.0 / (TYPE)273.0, (TYPE)4.0 / (TYPE)273.0},
                            {(TYPE)7.0 / (TYPE)273.0, (TYPE)26.0 / (TYPE)273.0, (TYPE)41.0 / (TYPE)273.0, (TYPE)26.0 / (TYPE)273.0, (TYPE)7.0 / (TYPE)273.0},
                            {(TYPE)4.0 / (TYPE)273.0, (TYPE)16.0 / (TYPE)273.0, (TYPE)26.0 / (TYPE)273.0, (TYPE)16.0 / (TYPE)273.0, (TYPE)4.0 / (TYPE)273.0},
                            {(TYPE)1.0 / (TYPE)273.0,  (TYPE)4.0 / (TYPE)273.0,  (TYPE)7.0 / (TYPE)273.0,  (TYPE)4.0 / (TYPE)273.0, (TYPE)1.0 / (TYPE)273.0} };
TYPE filter_host[K][K] = { {(TYPE)1.0 / (TYPE)273.0,  (TYPE)4.0 / (TYPE)273.0,  (TYPE)7.0 / (TYPE)273.0,  (TYPE)4.0 / (TYPE)273.0, (TYPE)1.0 / (TYPE)272.0},
                            {(TYPE)4.0 / (TYPE)273.0, (TYPE)16.0 / (TYPE)273.0, (TYPE)26.0 / (TYPE)273.0, (TYPE)16.0 / (TYPE)273.0, (TYPE)4.0 / (TYPE)273.0},
                            {(TYPE)7.0 / (TYPE)273.0, (TYPE)26.0 / (TYPE)273.0, (TYPE)41.0 / (TYPE)273.0, (TYPE)26.0 / (TYPE)273.0, (TYPE)7.0 / (TYPE)273.0},
                            {(TYPE)4.0 / (TYPE)273.0, (TYPE)16.0 / (TYPE)273.0, (TYPE)26.0 / (TYPE)273.0, (TYPE)16.0 / (TYPE)273.0, (TYPE)4.0 / (TYPE)273.0},
                            {(TYPE)1.0 / (TYPE)273.0,  (TYPE)4.0 / (TYPE)273.0,  (TYPE)7.0 / (TYPE)273.0,  (TYPE)4.0 / (TYPE)273.0, (TYPE)1.0 / (TYPE)273.0} };
 
//__global__ void TwoDConv(TYPE *img, TYPE* blur, TYPE* filter)
__global__ void TwoDConv(TYPE *img, TYPE* blur)
{
	int stride = blockDim.x * gridDim.x;
	int index  = blockIdx.x * blockDim.x + threadIdx.x;
    for (int i = index; i < SIZE; i += stride ) {
        for (int j = 0; j < SIZE; j++) {
            for (int k = -K/2; k < K/2; k++) {
				// boundary conditions will "reflect" the pixels as the filter steps outside the image boundaries
				int f_row = k+i < 0 ? -k : k;
				f_row     = k+i > SIZE-1 ? -k : k;
				for( int l = -K/2; l < K/2; l++ ) {
					int f_col = j+l < 0 ? -l : l;
					f_col     = j+l > SIZE-1 ? -l : l;
            		blur[i*SIZE+j]+= img[(i+f_row)*SIZE + j+f_col] * filter_device[k+K/2][l+K/2];
            		//blur[i*SIZE+j]+= img[(i+f_row)*SIZE + j+f_col];// * filter_device[k+K/2][l+K/2];
				}
            }
        }
    }
}

int main()
{
	TYPE* A = (TYPE*)malloc(sizeof(TYPE)*SIZE*SIZE);
	TYPE* blur = (TYPE*)malloc(sizeof(TYPE)*SIZE*SIZE);
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            A[i*SIZE+j] = fmod(rand(), 256); // simulate saturation values
            blur[i*SIZE+j] = 0;
        }
    }

    // create a stream
    cudaStream_t stream = NULL;
    CUDA_CHECK(cudaStreamCreateWithFlags(&stream, cudaStreamNonBlocking));
    // copy data to device
    TYPE* d_A;
    //TYPE* d_filter;
    TYPE* d_blur;
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void **>(&d_A), sizeof(TYPE) * SIZE*SIZE));
    //CUDA_CHECK(cudaMalloc(reinterpret_cast<void **>(&filter[0][0]), sizeof(TYPE) * K*K));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void **>(&d_blur), sizeof(TYPE) * SIZE*SIZE));
    CUDA_CHECK(cudaMemcpyAsync((void*)d_A, A, sizeof(TYPE) * SIZE*SIZE, cudaMemcpyHostToDevice, stream));
    //CUDA_CHECK(cudaMemcpyAsync((void*)d_filter, &filter[0][0], sizeof(TYPE) * K*K, cudaMemcpyHostToDevice, stream));
	// run 2DConv
	__TIMINGLIB_benchmark( [&] {
		//TwoDConv<<< (SIZE + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK, THREADS_PER_BLOCK >>>(d_A, d_blur, d_filter); 
		TwoDConv<<< (SIZE + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK, THREADS_PER_BLOCK >>>(d_A, d_blur); 
    	CUDA_CHECK(cudaStreamSynchronize(stream));
		CUDA_CHECK(cudaDeviceSynchronize());
	});
    // copy data to host
    CUDA_CHECK(cudaMemcpyAsync(blur, d_blur, sizeof(TYPE) * SIZE*SIZE, cudaMemcpyDeviceToHost, stream));

#if CHECK
	TYPE* C = (TYPE*)calloc(SIZE*SIZE, sizeof(TYPE));
	for( int i = 0; i < SIZE; i++ ) {
		for( int j = 0; j < SIZE; j++ ) {
			for( int k = -K/2; k < K/2; k++ ) {
				int f_row = i+k < 0 ? -k : k;
				    f_row = i+k > SIZE-1 ? -k : k;
				for( int l = -K/2; l < K/2; l++ ) {
					int f_col = j+l < 0 ? -l : l;
					    f_col = j+l > SIZE-1 ? -l : l;
					C[i*SIZE+j] += A[(i+f_row)*SIZE + (j+f_col)] * filter_host[k+K/2][l+K/2];
				}
			}
		}
	}
	__TIMINGLIB_snr(C, blur, sizeof(TYPE), SIZE*SIZE);
	free(C);
#endif

	free(A);
	free(blur);
    cudaFree(d_A);
    cudaFree(d_blur);
    cudaStreamDestroy(stream);
    cudaDeviceReset();
    return 0;
}
