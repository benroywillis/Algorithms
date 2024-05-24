// code example from https://www.goldsborough.me/cuda/ml/cudnn/c++/2017/10/01/14-37-23-convolutions_with_cudnn/

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <math.h>
#include <iostream>
#include <cudnn.h>
#include <cuda_runtime.h>

#include "TimingLib.h"

#if PRECISION == 0
#define TYPE float
#else
#define TYPE double 
#endif

// input image size - the image is simulated to be square
#ifndef SIZE
#define SIZE 512
#endif

// kernel size - it's also a square
#define K	5

#define CUDA_CHECK(err)                                                       \
    do {                                                                      \
        cudaError_t err_ = (err);                                             \
        if (err_ != cudaSuccess) {                                            \
            std::printf("CUDA error %d at %s:%d\n", err_, __FILE__, __LINE__);\
            std::exit(EXIT_FAILURE);                                          \
        }                                                                     \
    } while (0)

#define checkCUDNN(expr) {                                   \
    cudnnStatus_t status = (expr);                           \
    if (status != CUDNN_STATUS_SUCCESS) {                    \
      std::cerr << "Error on line " << __LINE__ << ": "      \
                << cudnnGetErrorString(status) << std::endl; \
      std::exit(EXIT_FAILURE);                               \
    }                                                        \
}

// discrete approximation for a 5x5 gaussian filter with mu = 0 and sigma = 1
__device__ TYPE d_filter[K][K] = { {(TYPE)1.0 / (TYPE)273.0,  (TYPE)4.0 / (TYPE)273.0,  (TYPE)7.0 / (TYPE)273.0,  (TYPE)4.0 / (TYPE)273.0, (TYPE)1.0 / (TYPE)272.0},
                            {(TYPE)4.0 / (TYPE)273.0, (TYPE)16.0 / (TYPE)273.0, (TYPE)26.0 / (TYPE)273.0, (TYPE)16.0 / (TYPE)273.0, (TYPE)4.0 / (TYPE)273.0},
                            {(TYPE)7.0 / (TYPE)273.0, (TYPE)26.0 / (TYPE)273.0, (TYPE)41.0 / (TYPE)273.0, (TYPE)26.0 / (TYPE)273.0, (TYPE)7.0 / (TYPE)273.0},
                            {(TYPE)4.0 / (TYPE)273.0, (TYPE)16.0 / (TYPE)273.0, (TYPE)26.0 / (TYPE)273.0, (TYPE)16.0 / (TYPE)273.0, (TYPE)4.0 / (TYPE)273.0},
                            {(TYPE)1.0 / (TYPE)273.0,  (TYPE)4.0 / (TYPE)273.0,  (TYPE)7.0 / (TYPE)273.0,  (TYPE)4.0 / (TYPE)273.0, (TYPE)1.0 / (TYPE)273.0} };
TYPE filter_host[K][K] = { {(TYPE)1.0 / (TYPE)273.0,  (TYPE)4.0 / (TYPE)273.0,  (TYPE)7.0 / (TYPE)273.0,  (TYPE)4.0 / (TYPE)273.0, (TYPE)1.0 / (TYPE)272.0},
                            {(TYPE)4.0 / (TYPE)273.0, (TYPE)16.0 / (TYPE)273.0, (TYPE)26.0 / (TYPE)273.0, (TYPE)16.0 / (TYPE)273.0, (TYPE)4.0 / (TYPE)273.0},
                            {(TYPE)7.0 / (TYPE)273.0, (TYPE)26.0 / (TYPE)273.0, (TYPE)41.0 / (TYPE)273.0, (TYPE)26.0 / (TYPE)273.0, (TYPE)7.0 / (TYPE)273.0},
                            {(TYPE)4.0 / (TYPE)273.0, (TYPE)16.0 / (TYPE)273.0, (TYPE)26.0 / (TYPE)273.0, (TYPE)16.0 / (TYPE)273.0, (TYPE)4.0 / (TYPE)273.0},
                            {(TYPE)1.0 / (TYPE)273.0,  (TYPE)4.0 / (TYPE)273.0,  (TYPE)7.0 / (TYPE)273.0,  (TYPE)4.0 / (TYPE)273.0, (TYPE)1.0 / (TYPE)273.0} };


int main(int argc, char *argv[]) {
	// step 0: make some data
	TYPE* img  = (TYPE*)malloc(SIZE*SIZE*sizeof(TYPE));
	TYPE* blur = (TYPE*)calloc(SIZE*SIZE,sizeof(TYPE));
	for( unsigned i = 0; i < SIZE; i++ ) {
		for( unsigned j = 0; j < SIZE; j++ ) {
			img[i*SIZE+j] = fmod((TYPE)rand(), 256);
		}
	}
    // step 1: create cudnn handle
    cudnnHandle_t cudnnH = NULL;
   	checkCUDNN(cudnnCreate(&cudnnH));

	// step 2: describe the convolution that will be going on
	// - we are doing a grayscale-image convolution, which has a single saturation channel of (simulated) uint8_t values
	cudnnTensorDescriptor_t input_descriptor;
	checkCUDNN(cudnnCreateTensorDescriptor(&input_descriptor));
	// input parameter descriptions can be found at https://docs.nvidia.com/deeplearning/cudnn/latest/api/cudnn-graph-library.html
	checkCUDNN(cudnnSetTensor4dDescriptor(input_descriptor,
                                      /*this format=*/CUDNN_TENSOR_NHWC,
                                      /*dataType=*/CUDNN_DATA_FLOAT,
                                      /*batch_size=*/1,
                                      /*channels=*/2,
                                      /*image_height=*/SIZE,
                                      /*image_width=*/SIZE));

	cudnnTensorDescriptor_t output_descriptor;
	checkCUDNN(cudnnCreateTensorDescriptor(&output_descriptor));
	checkCUDNN(cudnnSetTensor4dDescriptor(output_descriptor,
                                      /*format=*/CUDNN_TENSOR_NHWC,
                                      /*dataType=*/CUDNN_DATA_FLOAT,
                                      /*batch_size=*/1,
                                      /*channels=*/2,
                                      /*image_height=*/SIZE,
                                      /*image_width=*/SIZE));

	cudnnFilterDescriptor_t kernel_descriptor;
	checkCUDNN(cudnnCreateFilterDescriptor(&kernel_descriptor));
	checkCUDNN(cudnnSetFilter4dDescriptor(kernel_descriptor,
                                      /*dataType=*/CUDNN_DATA_FLOAT,
                                      /*format=*/CUDNN_TENSOR_NCHW,
                                      /*out_channels=*/2,
                                      /*in_channels=*/2,
                                      /*kernel_height=*/K,
                                      /*kernel_width=*/K));

	cudnnConvolutionDescriptor_t convolution_descriptor;
	checkCUDNN(cudnnCreateConvolutionDescriptor(&convolution_descriptor));
	checkCUDNN(cudnnSetConvolution2dDescriptor(convolution_descriptor,
                                           /*pad_height=*/1,
                                           /*pad_width=*/1,
                                           /*vertical_stride=*/1,
                                           /*horizontal_stride=*/1,
                                           /*dilation_height=*/1,
                                           /*dilation_width=*/1,
                                           /*mode=*/CUDNN_CROSS_CORRELATION,
                                           /*computeType=*/CUDNN_DATA_FLOAT));

	cudnnConvolutionFwdAlgo_t convolution_algorithm;
	checkCUDNN(cudnnGetConvolutionForwardAlgorithm(cudnnH,
                                        	input_descriptor,
                                        	kernel_descriptor,
                                        	convolution_descriptor,
                                        	output_descriptor,
                                        	CUDNN_CONVOLUTION_FWD_PREFER_FASTEST,
                                        	/*memoryLimitInBytes=*/0,
                                        	&convolution_algorithm));

	size_t workspace_bytes = 0;
	checkCUDNN(cudnnGetConvolutionForwardWorkspaceSize(cudnnH,
                                                   input_descriptor,
                                                   kernel_descriptor,
                                                   convolution_descriptor,
                                                   output_descriptor,
                                                   convolution_algorithm,
                                                   &workspace_bytes));
	std::cerr << "Workspace size: " << (workspace_bytes / 1048576.0) << "MB" << std::endl;

    // step 3: copy data to device
	void* d_workspace;
	TYPE* d_img;
	TYPE* d_blur;
	CUDA_CHECK(cudaMalloc(&d_workspace, workspace_bytes));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void **>(&d_img), sizeof(TYPE) * SIZE*SIZE));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void **>(&d_blur), sizeof(TYPE) * SIZE*SIZE));

    CUDA_CHECK(cudaMemcpy((void*)d_img, img, sizeof(TYPE) * SIZE*SIZE, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy((void*)d_blur, blur, sizeof(TYPE) * SIZE*SIZE, cudaMemcpyHostToDevice));

    // step 3: compute
    // these calls do not transpose the matrix (CUBLAS_OP_T would, CUBLAS_OP_C would be a hermitian transpose)
    const TYPE alpha = 1.0;
    const TYPE beta = 0.0;
	__TIMINGLIB_benchmark( [&] { 
#if PRECISION == 0
	const float alpha = 1, beta = 0;
	checkCUDNN(cudnnConvolutionForward(cudnnH,
                                   &alpha,
                                   input_descriptor,
                                   d_img,
                                   kernel_descriptor,
                                   d_filter,
                                   convolution_descriptor,
                                   convolution_algorithm,
                                   d_workspace,
                                   workspace_bytes,
                                   &beta,
                                   output_descriptor,
                                   d_blur));
#else
#endif
		cudaDeviceSynchronize();
	});

    // step 4: copy data to host
    CUDA_CHECK(cudaMemcpy(blur, d_blur, sizeof(TYPE) * SIZE*SIZE, cudaMemcpyDeviceToHost));
	// step 5: snr with CPU result
#if CHECK
	TYPE* D = (TYPE*)calloc(SIZE*SIZE, sizeof(TYPE));
	for( unsigned i = 0; i < SIZE; i++ ) {
		for( unsigned j = 0; j < SIZE; j++ ) {
			for( unsigned k = 0; k < SIZE; k++ ) {
				D[i*SIZE+j] += A[i*SIZE+k]*B[k*SIZE+j];
			}
		}
	}
	double num = 0.0;
	double den = 0.0;
	// the reference signal is the CPU result (D)
	for( unsigned i = 0; i < SIZE; i++ ) {
		for( unsigned j = 0; j < SIZE; j++ ) {
			num +=  D[i*SIZE+j]*D[i*SIZE+j];
			den += (D[i*SIZE+j]-C[i*SIZE+j])*(D[i*SIZE+j]-C[i*SIZE+j]);
		}
	}
    if (den < 0.001) printf("Reference and test outputs matched exactly\n");
    else 		     printf(" num: %g den: %g\n", num, den); printf("SNR: %.2fdb\n", 10.0*log10(num/den));
	free(D);
#endif

    // free resources
	free(img);
	free(blur);
    CUDA_CHECK(cudaFree(d_workspace));
    CUDA_CHECK(cudaFree(d_img));
    CUDA_CHECK(cudaFree(d_blur));
	checkCUDNN(cudnnDestroyTensorDescriptor(input_descriptor));
	checkCUDNN(cudnnDestroyTensorDescriptor(output_descriptor));
	checkCUDNN(cudnnDestroyFilterDescriptor(kernel_descriptor));
	checkCUDNN(cudnnDestroyConvolutionDescriptor(convolution_descriptor));

	checkCUDNN(cudnnDestroy(cudnnH));

    checkCUDNN(cudnnDestroy(cudnnH));
    CUDA_CHECK(cudaDeviceReset());

    return EXIT_SUCCESS;
}
