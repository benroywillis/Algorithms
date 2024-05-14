/*
 * Copyright 2020 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO LICENSEE:
 *
 * This source code and/or documentation ("Licensed Deliverables") are
 * subject to NVIDIA intellectual property rights under U.S. and
 * international Copyright laws.
 *
 * These Licensed Deliverables contained herein is PROPRIETARY and
 * CONFIDENTIAL to NVIDIA and is being provided under the terms and
 * conditions of a form of NVIDIA software license agreement by and
 * between NVIDIA and Licensee ("License Agreement") or electronically
 * accepted by Licensee.  Notwithstanding any terms or conditions to
 * the contrary in the License Agreement, reproduction or disclosure
 * of the Licensed Deliverables to any third party without the express
 * written consent of NVIDIA is prohibited.
 *
 * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
 * LICENSE AGREEMENT, NVIDIA MAKES NO REPRESENTATION ABOUT THE
 * SUITABILITY OF THESE LICENSED DELIVERABLES FOR ANY PURPOSE.  IT IS
 * PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY OF ANY KIND.
 * NVIDIA DISCLAIMS ALL WARRANTIES WITH REGARD TO THESE LICENSED
 * DELIVERABLES, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY,
 * NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
 * LICENSE AGREEMENT, IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY
 * SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THESE LICENSED DELIVERABLES.
 *
 * U.S. Government End Users.  These Licensed Deliverables are a
 * "commercial item" as that term is defined at 48 C.F.R. 2.101 (OCT
 * 1995), consisting of "commercial computer software" and "commercial
 * computer software documentation" as such terms are used in 48
 * C.F.R. 12.212 (SEPT 1995) and is provided to the U.S. Government
 * only as a commercial end item.  Consistent with 48 C.F.R.12.212 and
 * 48 C.F.R. 227.7202-1 through 227.7202-4 (JUNE 1995), all
 * U.S. Government End Users acquire the Licensed Deliverables with
 * only those rights set forth herein.
 *
 * Any use of the Licensed Deliverables in individual and commercial
 * software must include, in the user documentation and internal
 * comments to the code, the above Disclaimer and U.S. Government End
 * Users Notice.
 */

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <math.h>

#include <cublas_v2.h>
#include <cuda_runtime.h>

#include "cublas_utils.h"
#include "TimingLib.h"

#if PRECISION == 0
#define TYPE float
#else
#define TYPE double 
#endif

#ifndef SIZE
#define SIZE 512
#endif

int main(int argc, char *argv[]) {
	// step 0: make some data
	TYPE* A = (TYPE*)malloc(SIZE*SIZE*sizeof(TYPE));
	TYPE* B = (TYPE*)malloc(SIZE*SIZE*sizeof(TYPE));
	TYPE* C = (TYPE*)malloc(SIZE*SIZE*sizeof(TYPE));
	for( unsigned i = 0; i < SIZE; i++ ) {
		for( unsigned j = 0; j < SIZE; j++ ) {
			A[i*SIZE+j] = fmod((TYPE)rand(), SIZE);
			B[i*SIZE+j] = fmod((TYPE)rand(), SIZE);
			C[i*SIZE+j] = (TYPE)0.0;
		}
	}
    // step 1: create cublas handle, bind a stream
    cublasHandle_t cublasH = NULL;
    cudaStream_t stream = NULL;
    CUBLAS_CHECK(cublasCreate(&cublasH));

    CUDA_CHECK(cudaStreamCreateWithFlags(&stream, cudaStreamNonBlocking));
    CUBLAS_CHECK(cublasSetStream(cublasH, stream));

    // step 2: copy data to device
	TYPE* d_A;
	TYPE* d_B;
	TYPE* d_C;
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void **>(&d_A), sizeof(TYPE) * SIZE*SIZE));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void **>(&d_B), sizeof(TYPE) * SIZE*SIZE));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void **>(&d_C), sizeof(TYPE) * SIZE*SIZE));

	// BW 2024-05-09 No significant different in performance has been observed between Async transfers and Sync transfers (thus there seems to be a block of some sort for the Async case)
    CUDA_CHECK(cudaMemcpyAsync((void*)d_A, A, sizeof(TYPE) * SIZE*SIZE, cudaMemcpyHostToDevice, stream));
    CUDA_CHECK(cudaMemcpyAsync((void*)d_B, B, sizeof(TYPE) * SIZE*SIZE, cudaMemcpyHostToDevice, stream));
    //CUDA_CHECK(cudaMemcpy((void*)d_A, A, sizeof(TYPE) * SIZE*SIZE, cudaMemcpyHostToDevice));
    //CUDA_CHECK(cudaMemcpy((void*)d_B, B, sizeof(TYPE) * SIZE*SIZE, cudaMemcpyHostToDevice));

    // step 3: compute
    // these calls do not transpose the matrix (CUBLAS_OP_T would, CUBLAS_OP_C would be a hermitian transpose)
    const TYPE alpha = 1.0;
    const TYPE beta = 0.0;
	__TIMINGLIB_benchmark( [&] { 
#if PRECISION == 0
    	CUBLAS_CHECK(cublasSgemm(cublasH, CUBLAS_OP_N, CUBLAS_OP_N, SIZE, SIZE, SIZE, &alpha, d_A, SIZE, d_B, SIZE, &beta, d_C, SIZE));
#else
    	CUBLAS_CHECK(cublasDgemm(cublasH, CUBLAS_OP_N, CUBLAS_OP_N, SIZE, SIZE, SIZE, &alpha, d_A, SIZE, d_B, SIZE, &beta, d_C, SIZE));
#endif
    	CUDA_CHECK(cudaStreamSynchronize(stream));
		cudaDeviceSynchronize();
	});

    // step 4: copy data to host
    CUDA_CHECK(cudaMemcpyAsync(C, d_C, sizeof(TYPE) * SIZE*SIZE, cudaMemcpyDeviceToHost, stream));
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
#endif

    // free resources
	free(A);
	free(B);
	free(C);
    CUDA_CHECK(cudaFree(d_A));
    CUDA_CHECK(cudaFree(d_B));
    CUDA_CHECK(cudaFree(d_C));

    CUBLAS_CHECK(cublasDestroy(cublasH));

    CUDA_CHECK(cudaStreamDestroy(stream));

    CUDA_CHECK(cudaDeviceReset());

    return EXIT_SUCCESS;
}
