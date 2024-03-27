#include <iostream>
#include "TimingLib.h"

#if HALIDE_AUTOSCHEDULE == 1
#include "KernelGrammar_gemm_autoschedule_true_generated.h"
#endif
#include "KernelGrammar_gemm_autoschedule_false_generated.h"

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

	// USER: if you have any special reading functions for your inputs, inject them here and pass those parameters to the runtime buffers listed below (i.e., replace "nullptr" with your pointers)
	Runtime::Buffer<double> input0( nullptr, 1024, 1024 );
	input0.allocate();
	Runtime::Buffer<double> input1( nullptr, 1024, 1024 );
	input1.allocate();
	Runtime::Buffer<double> input2( nullptr, 1024, 1024 );
	input2.allocate();
	Runtime::Buffer<double> output0( nullptr, 1024, 1024);
	output0.allocate();

#if HALIDE_AUTOSCHEDULE == 1
	double autotime = __TIMINGLIB_benchmark([&]() {
		auto out = KernelGrammar_gemm_autoschedule_true_generated(input1, input2, output0);
		output0.device_sync();
		output0.copy_to_host();
	});
#endif

	/*double time = __TIMINGLIB_benchmark([&]() {
		auto out = KernelGrammar_gemm_autoschedule_false_generated(input1, input2, output0);
		output0.device_sync();
		output0.copy_to_host();
	});*/
	cout << "Success!" << endl;
	return 0;
}
