#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <float.h>
//#ifndef NO_AUTO_SCHEDULE
//#include "Debayer_autoschedule_true_generated.h"
#include "Debayer_autoschedule_false_generated.h"
//#endif

#include "HalideBuffer.h"
#include "halide_benchmark.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

// PERFECT methods for reading data
#include "wami_debayer.h"
#include "wami_utils.h"

int main(int argc, char **argv)
{
	halide_set_num_threads(1);
	if( argc != 4 )
	{
		std::cout << "Please provide input filename, directory, outputFilePath" << std::endl;
		return 1;
	}
	char* input_directory = argv[1];
	char* input_filename = argv[2];
    int timing_iterations = 15;
	// file load from PERFECT benchmark tools
    u16 (*bayer)[WAMI_DEBAYER_IMG_NUM_COLS] = NULL;
    rgb_pixel (*debayer)[WAMI_DEBAYER_IMG_NUM_COLS-2*PAD] = NULL;
#ifdef ENABLE_CORRECTNESS_CHECKING
    rgb_pixel (*gold_debayer)[WAMI_DEBAYER_IMG_NUM_COLS-2*PAD] = NULL;
#endif

    const size_t num_bayer_pixels = WAMI_DEBAYER_IMG_NUM_ROWS *
        WAMI_DEBAYER_IMG_NUM_COLS;
    const size_t num_debayer_pixels = (WAMI_DEBAYER_IMG_NUM_ROWS-2*PAD) *
        (WAMI_DEBAYER_IMG_NUM_COLS-2*PAD);

    bayer = (u16 (*)[WAMI_DEBAYER_IMG_NUM_COLS])malloc(sizeof(u16) * num_bayer_pixels);
    debayer = (rgb_pixel(*)[WAMI_DEBAYER_IMG_NUM_COLS-2*PAD])malloc(sizeof(rgb_pixel) * num_debayer_pixels);

    read_image_file(
        (char *) bayer,
        input_filename,
        input_directory,
        sizeof(u16) * num_bayer_pixels);

    memset(debayer, 0, sizeof(u16) * num_debayer_pixels);

 	halide_dimension_t bayerDim[] = {{0, WAMI_GMM_IMG_NUM_ROWS, 1} , {0, WAMI_GMM_IMG_NUM_COLS, 1}};
    Buffer<u16> Buffer_bayer( (u16*)bayer, 2, bayerDim );
 	halide_dimension_t debayerDim[] = {{0, WAMI_GMM_IMG_NUM_ROWS-2*PAD, 1} ,{0, WAMI_GMM_IMG_NUM_COLS-2*PAD, 1} ,{0, 3, 1}};
    Buffer<u16> Buffer_debayer( (u16*)debayer, 3, debayerDim);

    // Manually-tuned version
    double min_t_manual = benchmark(timing_iterations, 10, [&]() {
        Debayer_autoschedule_false_generated(Buffer_bayer, Buffer_debayer);
        Buffer_bayer.device_sync();
    });
    printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);

    // Auto-scheduled version
	// Ben [9/06/20] for some reason the autoscheduled version has an improper access to the output image and fails
    /*double min_t_auto = benchmark(timing_iterations, 10, [&]() {
	Debayer_autoschedule_true_generated(input, r_sigma, output);
        output.device_sync();
    });
    printf("Auto-scheduled time: %gms\n", min_t_auto * 1e3);*/

    //convert_and_save_image(output, argv[6]);

    return 0;
}
