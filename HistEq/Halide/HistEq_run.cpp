#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <float.h>
//#ifndef NO_AUTO_SCHEDULE
//#include "HistEq_autoschedule_true_generated.h"
#include "HistEq_autoschedule_false_generated.h"
//#endif

#include "HalideBuffer.h"
#include "halide_benchmark.h"
#include "halide_image_io.h"

// PERFECT methods for reading data
#include "octave/octave.h"
#define N_BINS 1 << 16

#include "TimingLib.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
	if( argc != 6 )
	{
		std::cout << "Please provide n_rows, n_cols, inputFilePath, outputFilePath, numThreads" << std::endl;
		return 1;
	}
	halide_set_num_threads(std::stoi(argv[5]));
	int n_rows = std::stoi(argv[1]);
	int n_cols = std::stoi(argv[2]);
	// file load from PERFECT benchmark tools
	int data[n_rows][n_cols];
	int PERFECT_answer[n_rows][n_cols];
	int err = read_array_from_octave((int*)&data, n_rows, n_cols, argv[3]);

 	halide_dimension_t arrayDims[] = {{0, n_rows, 1} , {0, n_cols, 1}};
    Buffer<int> input( (int*)&data[0][0], 2, arrayDims );
    Buffer<int> output(input.width(), input.height());

    // Manually-tuned version
    __TIMINGLIB_benchmark( [&]() {
        HistEq_autoschedule_false_generated(input, N_BINS, output);
        output.device_sync();
    });

    // Auto-scheduled version
	// Ben [9/06/20] for some reason the autoscheduled version has an improper access to the output image and fails
    /*double min_t_auto = benchmark(timing_iterations, 10, [&]() {
	HistEq_autoschedule_true_generated(input, r_sigma, output);
        output.device_sync();
    });
    printf("Auto-scheduled time: %gms\n", min_t_auto * 1e3);*/

    //convert_and_save_image(output, argv[4]);

	// compare to PERFECT output
	/*int badMatch = 0;
	err = read_array_from_octave((int*)&PERFECT_answer, n_rows, n_cols, "../PERFECT/histeq_output.small.0.mat");
	for( int i = 0; i < n_rows; i++ )
	{
		for( int j = 0; j < n_cols; j++ )
		{
			if( (PERFECT_answer[i][j] != output(j, i)) )
			{
				badMatch++;
				printf("%d, %d\n", output(j, i), PERFECT_answer[i][j]);
			}
		}
	}
	printf("%.2f%% of the output did not match\n", (float)((float)badMatch/(float)(n_rows*n_cols)*100) );*/

    return 0;
}
