#include <iostream>
#include <cstdint>
#include "TimingLib.h"

#if HALIDE_AUTOSCHEDULE == 1
#include "KernelGrammar_Hist_autoschedule_true_generated.h"
#endif
#include "KernelGrammar_Hist_autoschedule_false_generated.h"

#include "HalideBuffer.h"

// all these things were added to parameterize the program (for performance comparisons)
#ifndef SIZE
#define SIZE 512
#endif

#ifndef PRECISION
#define TYPE uint32_t
#define TYPE_MAX 2147483647
#elif PRECISION == 0
#define TYPE uint32_t
#define TYPE_MAX 2147483647
#elif PRECISION == 1
#define TYPE uint16_t
#define TYPE_MAX 32767
#elif PRECISION == 2
#define TYPE uint8_t
#define TYPE_MAX 255
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
	TYPE* in  = (TYPE*)malloc( SIZE*SIZE*sizeof(TYPE) );
	for( unsigned i = 0; i < SIZE; i++ ) {
		for( unsigned j = 0; j < SIZE; j++ ) {
			in[i*SIZE+j] = (TYPE)rand() % TYPE_MAX;
		}
	}
	TYPE* out = (TYPE*)calloc( TYPE_MAX, sizeof(TYPE) );
	Runtime::Buffer<TYPE> input0( in, SIZE, SIZE );
	Runtime::Buffer<TYPE> output0( out, TYPE_MAX );
#if HALIDE_AUTOSCHEDULE == 1
	double autotime = __TIMINGLIB_benchmark([&]() {
		auto out = KernelGrammar_Hist_autoschedule_true_generated(input0, output0);
		output0.device_sync();
		output0.copy_to_host();
	});
#endif

	double time = __TIMINGLIB_benchmark([&]() {
		auto out = KernelGrammar_Hist_autoschedule_false_generated(input0, output0);
		output0.device_sync();
		output0.copy_to_host();
	});
	cout << "Success!" << endl;

#if CHECK
	TYPE* hist = (TYPE*)calloc( TYPE_MAX, sizeof(TYPE) );
	for( unsigned i = 0; i < SIZE; i++ ) {
		for( unsigned j = 0; j < SIZE; j++ ) {
			hist[ in[i*SIZE+j] ] += 1;
		}
	}
	__TIMINGLIB_snr(hist, out, sizeof(TYPE), TYPE_MAX);
	free(hist);
#endif
	free(in);
	free(out);
	return 0;
}
