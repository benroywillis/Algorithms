#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <float.h>
//#ifndef NO_AUTO_SCHEDULE
//#include "GMM_autoschedule_true_generated.h"
#include "GMM_autoschedule_false_generated.h"
//#endif

#include "HalideBuffer.h"
#include "halide_benchmark.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

// PERFECT methods for reading data
#include "wami_params.h"

void read_gmm_input_data(
    float mu[WAMI_GMM_IMG_NUM_ROWS][WAMI_GMM_IMG_NUM_COLS][WAMI_GMM_NUM_MODELS],
    float sigma[WAMI_GMM_IMG_NUM_ROWS][WAMI_GMM_IMG_NUM_COLS][WAMI_GMM_NUM_MODELS],
    float weights[WAMI_GMM_IMG_NUM_ROWS][WAMI_GMM_IMG_NUM_COLS][WAMI_GMM_NUM_MODELS],
    u16 frames[WAMI_GMM_NUM_FRAMES][WAMI_GMM_IMG_NUM_ROWS][WAMI_GMM_IMG_NUM_COLS],
    const char *filename,
    const char *directory);

int main(int argc, char **argv)
{
	if( argc != 4 )
	{
		std::cout << "Please provide input filename, directory, outputFilePath" << std::endl;
		return 1;
	}
	char* input_directory= argv[1];
	char* input_filename = argv[2];

    float (*mu)[WAMI_GMM_IMG_NUM_COLS][WAMI_GMM_NUM_MODELS] = NULL;
    float (*sigma)[WAMI_GMM_IMG_NUM_COLS][WAMI_GMM_NUM_MODELS] = NULL;
    float (*weights)[WAMI_GMM_IMG_NUM_COLS][WAMI_GMM_NUM_MODELS] = NULL;
    u8 (*foreground)[WAMI_GMM_IMG_NUM_ROWS][WAMI_GMM_IMG_NUM_COLS] = NULL;
#ifdef ENABLE_CORRECTNESS_CHECKING
    u8 (*golden_foreground)[WAMI_GMM_IMG_NUM_ROWS][WAMI_GMM_IMG_NUM_COLS] = NULL;
    u8 (*golden_eroded)[WAMI_GMM_IMG_NUM_COLS] = NULL;
    u8 (*eroded)[WAMI_GMM_IMG_NUM_COLS] = NULL;
#endif
    u8 (*morph)[WAMI_GMM_IMG_NUM_COLS] = NULL;
    u16 (*frames)[WAMI_GMM_IMG_NUM_ROWS][WAMI_GMM_IMG_NUM_COLS] = NULL;
    int i;
    const size_t num_pixels = WAMI_GMM_IMG_NUM_ROWS * WAMI_GMM_IMG_NUM_COLS;

    mu = (float (*)[WAMI_GMM_IMG_NUM_COLS][WAMI_GMM_NUM_MODELS])malloc(sizeof(float) * num_pixels * WAMI_GMM_NUM_MODELS);
    sigma = (float (*)[WAMI_GMM_IMG_NUM_COLS][WAMI_GMM_NUM_MODELS])malloc(sizeof(float) * num_pixels * WAMI_GMM_NUM_MODELS);
    weights = (float (*)[WAMI_GMM_IMG_NUM_COLS][WAMI_GMM_NUM_MODELS])malloc(sizeof(float) * num_pixels * WAMI_GMM_NUM_MODELS);
    foreground = (u8 (*)[WAMI_GMM_IMG_NUM_ROWS][WAMI_GMM_IMG_NUM_COLS])malloc(sizeof(u8) * num_pixels * WAMI_GMM_NUM_FRAMES);
#ifdef ENABLE_CORRECTNESS_CHECKING
    golden_foreground = malloc(sizeof(u8) * num_pixels * WAMI_GMM_NUM_FRAMES);
    eroded = malloc(sizeof(u8) * num_pixels);
    golden_eroded = malloc(sizeof(u8) * num_pixels);
#endif
    morph = (u8 (*)[WAMI_GMM_IMG_NUM_COLS])malloc(sizeof(u8) * num_pixels);
    frames = (u16 (*)[WAMI_GMM_IMG_NUM_ROWS][WAMI_GMM_IMG_NUM_COLS])malloc(sizeof(u16) * num_pixels * WAMI_GMM_NUM_FRAMES);

    memset(mu, 0, sizeof(float) * num_pixels * WAMI_GMM_NUM_MODELS);
    memset(sigma, 0, sizeof(float) * num_pixels * WAMI_GMM_NUM_MODELS);
    memset(weights, 0, sizeof(float) * num_pixels * WAMI_GMM_NUM_MODELS);
    memset(foreground, 0, sizeof(u8) * num_pixels * WAMI_GMM_NUM_FRAMES);
    memset(morph, 0, sizeof(u8) * num_pixels);
    memset(frames, 0, sizeof(u16) * num_pixels * WAMI_GMM_NUM_FRAMES);

    read_gmm_input_data(mu, sigma, weights, frames, input_filename, input_directory);

    int timing_iterations = 15;
	// file load from PERFECT benchmark tools

 	halide_dimension_t foregroundDims[] = {{0, WAMI_GMM_IMG_NUM_ROWS, 1} , {0, WAMI_GMM_IMG_NUM_COLS, 1}};
    Buffer<uint8_t> Buffer_foreground( (uint8_t*)foreground, 3, foregroundDims );
 	halide_dimension_t parameterDims[] = {{0, WAMI_GMM_IMG_NUM_ROWS, 1} , {0, WAMI_GMM_IMG_NUM_COLS, 1}, {0, WAMI_GMM_NUM_MODELS, 1}};
    Buffer<float> Buffer_mu( (float*)mu, 3, parameterDims);
    Buffer<float> Buffer_sigma( (float*)sigma, 3, parameterDims);
    Buffer<float> Buffer_weight( (float*)weights, 3, parameterDims);
 	halide_dimension_t frameDims[] = {{0, WAMI_GMM_NUM_FRAMES, 1} ,{0, WAMI_GMM_IMG_NUM_ROWS, 1} , {0, WAMI_GMM_IMG_NUM_COLS, 1}};
    Buffer<uint16_t> Buffer_frames( (uint16_t*)frames, 3, frameDims );

    // Manually-tuned version
    double min_t_manual = benchmark(timing_iterations, 10, [&]() {
        GMM_autoschedule_false_generated(Buffer_mu, Buffer_sigma, Buffer_weight, Buffer_frames, Buffer_foreground);
        Buffer_foreground.device_sync();
    });
    printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);

    // Auto-scheduled version
	// Ben [9/06/20] for some reason the autoscheduled version has an improper access to the output image and fails
    /*double min_t_auto = benchmark(timing_iterations, 10, [&]() {
	GMM_autoschedule_true_generated(input, r_sigma, output);
        output.device_sync();
    });
    printf("Auto-scheduled time: %gms\n", min_t_auto * 1e3);*/

    //convert_and_save_image(output, argv[6]);

    return 0;
}
