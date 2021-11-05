#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <float.h>
//#ifndef NO_AUTO_SCHEDULE
//#include "TwoDConv_autoschedule_true_generated.h"
#include "TwoDConv_autoschedule_false_generated.h"
//#endif

#include "HalideBuffer.h"
#include "halide_benchmark.h"
#include "halide_image_io.h"

// PERFECT methods for reading data
#include "octave/octave.h"
#define N_BINS 255

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
	halide_set_num_threads(1);
    int timing_iterations = 15;
	if( argc != 5 )
	{
		std::cout << "Please provide n_rows, n_cols, inputFilePath, outputFilePath" << std::endl;
		return 1;
	}
	int n_rows = std::stoi(argv[1]);
	int n_cols = std::stoi(argv[2]);
	// file load from PERFECT benchmark tools
	int data[n_rows][n_cols];
	int err = read_array_from_octave((int*)&data, n_rows, n_cols, argv[4]);

 	halide_dimension_t arrayDims[] = {{0, n_rows, 1} , {0, n_cols, 1}};
    Buffer<int> input( (int*)&data[0][0], 2, arrayDims );
    Buffer<int> output(input.width(), input.height());

    // Manually-tuned version
    double min_t_manual = benchmark(timing_iterations, 10, [&]() {
        TwoDConv_autoschedule_false_generated(input, output);
        output.device_sync();
    });
    printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);

    // Auto-scheduled version
	// Ben [9/06/20] for some reason the autoscheduled version has an improper access to the output image and fails
    /*double min_t_auto = benchmark(timing_iterations, 10, [&]() {
	TwoDConv_autoschedule_true_generated(input, r_sigma, output);
        output.device_sync();
    });
    printf("Auto-scheduled time: %gms\n", min_t_auto * 1e3);*/

    convert_and_save_image(output, argv[4]);

    return 0;
}
