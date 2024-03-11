#include <iostream>
#include "TimingLib.h"
// for reading in the images like the user did
#include "BilateralFilter.h"

#if HALIDE_AUTOSCHEDULE == 1
#include "KernelGrammar_StencilChain_autoschedule_true_generated.h"
#endif
#include "KernelGrammar_StencilChain_autoschedule_false_generated.h"

#include "HalideBuffer.h"

using namespace std;
using namespace Halide;

int main(int argc, char** argv) {
	if( argc != 4 ) {
		cout << "Usage: <input_image> <output_image> threads<int>" << endl;
		return 1;
	}
	int threads = stoi(argv[3]);
	cout << "Setting thread count to "+to_string(threads) << endl;
	halide_set_num_threads(threads);

	// USER: if you have any special reading functions for your inputs, inject them here and pass those parameters to the runtime buffers listed below (i.e., replace "nullptr" with your pointers)
    struct Pixel* input  = readImage(argv[1]);
	uint32_t* input_converted = (uint32_t*)malloc( image_width * image_height * 3 * sizeof(uint32_t) );
	for( unsigned y = 0; y < image_height; y++ ) {
		for( unsigned x = 0; x < image_width; x++ ) {
			input_converted[y*image_height + 3*x] = (uint32_t)input[y*image_height+x].r;
			input_converted[y*image_height + 3*x + 1] = (uint32_t)input[y*image_height+x].g;
			input_converted[y*image_height + 3*x + 2] = (uint32_t)input[y*image_height+x].b;
		}
	}
    printf("Image size: %d x %d\n", image_height, image_width);
	Runtime::Buffer<uint32_t> in( input_converted, image_height, image_width, 3 );
	Runtime::Buffer<float> output( nullptr, image_height, image_width );
	output.allocate();
		
#if HALIDE_AUTOSCHEDULE == 1
	double autotime = __TIMINGLIB_benchmark([&]() {
		auto out = KernelGrammar_StencilChain_autoschedule_true_generated(in, output);
	});
#endif

	/*double time = __TIMINGLIB_benchmark([&]() {
		auto out = KernelGrammar_StencilChain_autoschedule_false_generated();
	});*/
	cout << "Success!" << endl;
	return 0;
}
