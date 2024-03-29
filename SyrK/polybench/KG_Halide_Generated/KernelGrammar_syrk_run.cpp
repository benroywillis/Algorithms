#include <iostream>
#include "TimingLib.h"

#ifndef SIZE
#define SIZE 1024
#endif

#if HALIDE_AUTOSCHEDULE == 1
#include "KernelGrammar_syrk_autoschedule_true_generated.h"
#endif
#include "KernelGrammar_syrk_autoschedule_false_generated.h"

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
	double* in0 = (double*)malloc(SIZE*SIZE*sizeof(double));
	double* in1 = (double*)malloc(SIZE*SIZE*sizeof(double));
	double* out = (double*)malloc(SIZE*SIZE*sizeof(double));
	for( unsigned y = 0; y < SIZE; y++ ) {
		for( unsigned x = 0; x < SIZE; x++ ) {
			in0[y*SIZE+x] = rand();
			in1[y*SIZE+x] = rand();
			out[y*SIZE+x] = 0.0;
		}
	}
	Runtime::Buffer<double> input0(  in0, SIZE, SIZE );
	Runtime::Buffer<double> input1(  in1, SIZE, SIZE );
	Runtime::Buffer<double> output0( out, SIZE, SIZE );

#if HALIDE_AUTOSCHEDULE == 1
	double autotime = __TIMINGLIB_benchmark([&]() {
		auto out = KernelGrammar_syrk_autoschedule_true_generated(input0, input1, output0);
		output0.device_sync();
		output0.copy_to_host();
	});
#endif
	/*
	double time = __TIMINGLIB_benchmark([&]() {
		auto out = KernelGrammar_syrk_autoschedule_false_generated(input0, input1, output0);
		output0.device_sync();
		output0.copy_to_host();
	});*/
	cout << "Success!" << endl;
	return 0;
}
