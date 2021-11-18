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

#if INPUT_SIZE == INPUT_SIZE_SMALL
    static const char *input_filename = "small_kernel3_input.bin";
    static const char *golden_output_filename = "small_golden_kernel3_output.bin";
    static const char *output_filename = "small_kernel3_output.bin";
#elif INPUT_SIZE == INPUT_SIZE_MEDIUM
    static const char *input_filename = "medium_kernel3_input.bin";
    static const char *golden_output_filename = "medium_golden_kernel3_output.bin";
    static const char *output_filename = "medium_kernel3_output.bin";
#elif INPUT_SIZE == INPUT_SIZE_LARGE
    static const char *input_filename = "large_kernel3_input.bin";
    static const char *golden_output_filename = "large_golden_kernel3_output.bin";
    static const char *output_filename = "large_kernel3_output.bin";
#else
    #error "Unhandled value for INPUT_SIZE"
#endif

#define ENABLE_CORRECTNESS_CHECKING

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
#ifdef ENABLE_CORRECTNESS_CHECKING
    read_data_file(
        (char *) golden_foreground,
        golden_output_filename,
        input_directory,
        sizeof(u8) * num_pixels * WAMI_GMM_NUM_FRAMES);
#endif

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
#ifdef ENABLE_CORRECTNESS_CHECKING
    {
        int j, k, validation_warning = 0;
        for (i = 0; i < WAMI_GMM_NUM_FRAMES; ++i)
        {
            int num_misclassified = 0, num_foreground = 0;
            double misclassification_rate = 0;

            wami_morpho_erode(
                eroded, (u8 (*)[WAMI_GMM_IMG_NUM_COLS]) &foreground[i][0][0]);
            wami_morpho_erode(
                golden_eroded, (u8 (*)[WAMI_GMM_IMG_NUM_COLS]) &golden_foreground[i][0][0]);

            printf("\nValidating frame %d output...\n", i);

            for (j = 0; j < WAMI_GMM_IMG_NUM_ROWS; ++j)
            {
                for (k = 0; k < WAMI_GMM_IMG_NUM_COLS; ++k)
                {
                    if (eroded[j][k] != golden_eroded[j][k])
                    {
                        ++num_misclassified;
                    }
                    if (golden_eroded[j][k] != 0)
                    {
                        ++num_foreground;
                    }
                }
            }
            misclassification_rate = (100.0*num_misclassified)/num_foreground;
            printf("\tMisclassified pixels: %d\n", num_misclassified);
            printf("\tGolden foreground pixels (after erosion): %d\n", num_foreground);
            printf("\tMisclassification rate relative to foreground: %f%%\n",
                misclassification_rate);
            if (misclassification_rate > 0.1)
            {
                validation_warning = 1;
            }
        }
	if (validation_warning)
        {
            printf("\nValidation warning: Misclassification rate appears high; check images.\n\n");
        }
        else
        {
            printf("\nValidation checks passed.\n\n");
        }
    }
#endif

    FREE_AND_NULL(mu);
    FREE_AND_NULL(sigma);
    FREE_AND_NULL(weights);
    FREE_AND_NULL(foreground);
#ifdef ENABLE_CORRECTNESS_CHECKING
    FREE_AND_NULL(golden_foreground);
    FREE_AND_NULL(eroded);
    FREE_AND_NULL(golden_eroded);
#endif
    FREE_AND_NULL(morph);
    FREE_AND_NULL(frames);
   
    return 0;
}
