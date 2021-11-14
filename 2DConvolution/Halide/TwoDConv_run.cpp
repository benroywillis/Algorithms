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

using namespace Halide::Tools;
using namespace Halide::Runtime;

// PERFECT methods for reading data
#include "octave/octave.h"
#define N_BINS 255


#if !defined(BATCH_SIZE)
#define BATCH_SIZE (30)
#endif

#if INPUT_SIZE == INPUT_SIZE_SMALL
#define M 640  /* columns */
#define N 480  /* rows */
#define FILENAME "../PERFECT/input_small.mat"
#define SIZE "small"

#elif INPUT_SIZE == INPUT_SIZE_MEDIUM
#define M 1920  /* columns */
#define N 1080  /* rows */
#define FILENAME "../../../input/input_medium.mat"
#define SIZE "medium"

#elif INPUT_SIZE == INPUT_SIZE_LARGE
#define M 3840  /* columns */
#define N 2160  /* rows */
#define FILENAME "../../../input/input_large.mat"
#define SIZE "large"

#define PERFECT_answer_filename 

#else
#error "Unhandled value for INPUT_SIZE"
#endif

int main(int argc, char **argv) {
	std::string PERFECT_answer_filename = "../PERFECT/2dconv_output.small.";
	halide_set_num_threads(1);
	// file load from PERFECT benchmark tools
	int (*data)[M][N] = (int (*)[M][N])malloc(sizeof(int)*BATCH_SIZE*M*N);
	int (*output)[M][N] = (int (*)[M][N])malloc(sizeof(int)*BATCH_SIZE*M*N);
	int (*PERFECT_answer)[M][N] = (int (*)[M][N])malloc(sizeof(int)*BATCH_SIZE*M*N);
	int err = read_array_from_octave(&data[0][0][0], M, N, (char*)&FILENAME);
	for( int i = 1; i < BATCH_SIZE; i++ ) { 
		for( int j = 0; j < M; j++ ) {
			for( int k = 0; k < N; k++ ) {
				data[i][j][k] = data[i-1][j][k];
			}
		}
	}

    Buffer<int> Buffer_input( (int*)data, N, M, BATCH_SIZE);
    Buffer<int> Buffer_output((int*)output, N, M, BATCH_SIZE);

    // Manually-tuned version
    int timing_iterations = 1;
    double min_t_manual = benchmark(timing_iterations, 10, [&]() {
        TwoDConv_autoschedule_false_generated(Buffer_input, Buffer_output);
        Buffer_output.device_sync();
    });
    printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);

	uint64_t badMatch = 0;
	for( int i = 0; i < BATCH_SIZE; i++ )
	{
		for( int j = 0; j < M; j++ )
		{
			for( int k = 0; k < N; k++ )
			{
				if( Buffer_input(k, j, i) != data[i][j][k] )
				{
					badMatch++;
				}
			}
		}
	}
	printf("%.2f%% of the input did not match\n", (float)( (float)badMatch / ((float)BATCH_SIZE*M*N) * 100 ) );

	// read in the output of the PERFECT algorithm and see if they match ours
	for( int i = 0; i < BATCH_SIZE; i++ ) { 
		err = read_array_from_octave( (int*)&PERFECT_answer[i], M, N, (char*)std::string(PERFECT_answer_filename+std::to_string(i)+".mat").data());
	}
	for( int i = 0; i < BATCH_SIZE; i++ )
	{
		for( int j = 0; j < M; j++ )
		{
			for( int k = 0; k < N; k++ )
			{
				if( Buffer_output(k, j, i) != PERFECT_answer[i][j][k] )
				{
					badMatch++;
					printf(" %d, %d \n", Buffer_output(k, j, i), PERFECT_answer[i][j][k]);
				}
			}
		}
	}
	printf("%.2f%% of the output did not match\n", (float)( (float)badMatch / ((float)BATCH_SIZE*M*N) * 100 ) );

    // Auto-scheduled version
	// Ben [9/06/20] for some reason the autoscheduled version has an improper access to the output image and fails
    /*double min_t_auto = benchmark(timing_iterations, 10, [&]() {
	TwoDConv_autoschedule_true_generated(input, r_sigma, output);
        output.device_sync();
    });
    printf("Auto-scheduled time: %gms\n", min_t_auto * 1e3);*/

    //convert_and_save_image(Buffer_output, argv[4]);

    return 0;
}
