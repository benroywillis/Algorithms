#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <float.h>
//#ifndef NO_AUTO_SCHEDULE
//#include "LucasKanade_autoschedule_true_generated.h"
#include "LucasKanade_autoschedule_false_generated.h"
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
	if( argc != 6 )
	{
		std::cout << "Please provide n_rows, n_cols, input_Dx, input_Dy, outputFilePath" << std::endl;
		return 1;
	}
	int n_rows = std::stoi(argv[1]);
	int n_cols = std::stoi(argv[2]);
	// file load from PERFECT benchmark tools
	float* data_Dx = (float*)malloc(sizeof(float)*n_rows*n_cols);
	float* data_Dy = (float*)malloc(sizeof(float)*n_rows*n_cols);
	float* data2 = (float*)malloc(sizeof(float)*n_rows*n_cols*6);
    int err  = read_fltarray_from_octave (data_Dx, n_rows, n_cols, argv[4]);
    int err2 = read_fltarray_from_octave (data_Dy, n_rows, n_cols, argv[5]);

 	halide_dimension_t arrayDims[] = {{0, n_rows, 1} , {0, n_cols, 1}};
    Buffer<float> input_Dx( data_Dx, 2, arrayDims );
    Buffer<float> input_Dy( data_Dy, 2, arrayDims );
 	halide_dimension_t jacobianDims[] = { {0, 6*n_rows, 1} , {0, n_cols, 1}, {0, 6, 1} };
    Buffer<float> jacobian( data2, 3, jacobianDims );
    Buffer<float> output(6, 6);

    // Manually-tuned version
    double min_t_manual = benchmark(timing_iterations, 10, [&]() {
        LucasKanade_autoschedule_false_generated(input_Dx, input_Dy, jacobian, output);
        output.device_sync();
    });
    printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);

    // Auto-scheduled version
	// Ben [9/06/20] for some reason the autoscheduled version has an improper access to the output image and fails
    /*double min_t_auto = benchmark(timing_iterations, 10, [&]() {
	LucasKanade_autoschedule_true_generated(input, r_sigma, output);
        output.device_sync();
    });
    printf("Auto-scheduled time: %gms\n", min_t_auto * 1e3);*/

    //convert_and_save_image(output, argv[6]);

    return 0;
}
