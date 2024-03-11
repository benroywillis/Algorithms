#include <iostream>
#include "TimingLib.h"

#if HALIDE_AUTOSCHEDULE == 1
#include "KernelGrammar_ElementMultiply_autoschedule_true_generated.h"
#endif
#include "KernelGrammar_ElementMultiply_autoschedule_false_generated.h"

#include "HalideBuffer.h"

// 2024-03-08 Things done by the user
// 1. change input parameters to skip the inputs and just dynamically allocate random arrays
// 2. added SIZE parameter for speed tests
// 3. the template arguments in Runtime::Buffer<> were "ptr" and they need to be "float"

#ifndef SIZE
#define SIZE 512
#endif

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

	// USER: if you have any special reading functions for your inputs, inject them here and pass those parameters to the runtime buffers listed below (i.e., replace "nullptr" with your pointers)
	float* in0 = (float*)malloc( SIZE*SIZE*sizeof(float) );
	float* in1 = (float*)malloc( SIZE*SIZE*sizeof(float) );
	float* out = (float*)malloc( SIZE*SIZE*sizeof(float) );
	Runtime::Buffer<float> input0( in0, SIZE, SIZE);
	input0.allocate();
	Runtime::Buffer<float> input1( in1, SIZE, SIZE);
	input1.allocate();
	Runtime::Buffer<float> output( out, SIZE, SIZE);
	output.allocate();

#if HALIDE_AUTOSCHEDULE == 1
	double autotime = __TIMINGLIB_benchmark([&]() {
		auto out = KernelGrammar_ElementMultiply_autoschedule_true_generated(input0, input1, output);
	});
#endif

	double time = __TIMINGLIB_benchmark([&]() {
		auto out = KernelGrammar_ElementMultiply_autoschedule_false_generated(input0, input1, output);
	});
	cout << "Success!" << endl;
	return 0;
}
