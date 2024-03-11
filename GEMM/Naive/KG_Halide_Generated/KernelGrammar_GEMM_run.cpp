#include <iostream>
#include "TimingLib.h"

// user modifications
// 1. change input args to nothing
// 2. make rand init matrices and stuff them into the pipeline
// 3. make output matrix

#ifndef SIZE
#define SIZE 64
#endif

#if HALIDE_AUTOSCHEDULE == 1
#include "KernelGrammar_GEMM_autoschedule_true_generated.h"
#endif
#include "KernelGrammar_GEMM_autoschedule_false_generated.h"

#include "HalideBuffer.h"

using namespace std;
using namespace Halide;

int main(int argc, char** argv) {
	if( argc != 2 ) {
		cout << "Usage: threads<int>" << endl;
		return 1;
	}
	int threads = stoi(argv[1]);
	cout << "Setting thread count to "+to_string(threads) << endl;
	halide_set_num_threads(threads);

	float* in0 = (float*)malloc( SIZE*SIZE*sizeof(float) );
	float* in1 = (float*)malloc( SIZE*SIZE*sizeof(float) );
	float* out = (float*)malloc( SIZE*SIZE*sizeof(float) );

	for( unsigned y = 0; y < SIZE; y++ ) {
		for( unsigned x = 0; x < SIZE; x++ ) {
			in0[y*SIZE + x] = (float)rand();
			in1[y*SIZE + x] = (float)rand();
			out[y*SIZE + x] = 0.0f;
		}
	}

	// USER: if you have any special reading functions for your inputs, inject them here and pass those parameters to the runtime buffers listed below (i.e., replace "nullptr" with your pointers)
	Runtime::Buffer<float> input0( in0, SIZE, SIZE );
	Runtime::Buffer<float> input1( in1, SIZE, SIZE );
	Runtime::Buffer<float> output( out, SIZE, SIZE );

#if HALIDE_AUTOSCHEDULE == 1
	double autotime = __TIMINGLIB_benchmark([&]() {
		auto ret = KernelGrammar_GEMM_autoschedule_true_generated(input0, input1, output);
	});
#endif

	/*double time = __TIMINGLIB_benchmark([&]() {
		auto ret = KernelGrammar_GEMM_autoschedule_false_generated(input0, input1, output);
	});*/
	cout << "Success!" << endl;
	return 0;
}
