#include <iostream>
#include "TimingLib.h"

#if HALIDE_AUTOSCHEDULE == 1
#include "KernelGrammar_GEMV_autoschedule_true_generated.h"
#endif
#include "KernelGrammar_GEMV_autoschedule_false_generated.h"

#include "HalideBuffer.h"

using namespace std;
using namespace Halide;

// 2024-03-08 User annotations
// 1. implemented size parameter for speed testing
// 2. implemented array allocation and initialization (and changed the runtime args since they don't handle the input)
// 3. changed ptr template argument in Runtime::Buffer constructors to float
// 4. the inputs to the pipeline are in the wrong order 

int main(int argc, char** argv) {
	if( argc != 2 ) {
		cout << "Usage: threads<int>" << endl;
		return 1;
	}
	int threads = stoi(argv[1]);
	cout << "Setting thread count to "+to_string(threads) << endl;
	halide_set_num_threads(threads);

	// USER: if you have any special reading functions for your inputs, inject them here and pass those parameters to the runtime buffers listed below (i.e., replace "nullptr" with your pointers)
	float* in0 = (float*)malloc( SIZE*sizeof(float) );
	float* in1 = (float*)malloc( SIZE*SIZE*sizeof(float) );
	float* out = (float*)malloc( SIZE*sizeof(float) );
	Runtime::Buffer<float> input0( in0, SIZE);
	//input0.allocate();
	Runtime::Buffer<float> input1( in1, SIZE, SIZE);
	//input1.allocate();
	Runtime::Buffer<float> output0( out, SIZE);
	//output0.allocate();

#if HALIDE_AUTOSCHEDULE == 1
	double autotime = __TIMINGLIB_benchmark([&]() {
		//auto out = KernelGrammar_GEMV_autoschedule_true_generated(input0, input1, output0);
		auto out = KernelGrammar_GEMV_autoschedule_true_generated(input1, input0, output0);
		output0.device_sync();
		output0.copy_to_host();
	});
#endif

	double time = __TIMINGLIB_benchmark([&]() {
		//auto out = KernelGrammar_GEMV_autoschedule_false_generated(input0, input1, output0);
		auto out = KernelGrammar_GEMV_autoschedule_false_generated(input1, input0, output0);
		output0.device_sync();
		output0.copy_to_host();
	});
	cout << "Success!" << endl;
	return 0;
}
