#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
//#include "bilateral_filter.h"
//#ifndef NO_AUTO_SCHEDULE
//#include "bilateral_filter_autoschedule_true_generated.h"
#include "bilateral_filter_autoschedule_false_generated.h"
//#endif

#include "HalideBuffer.h"
#include "halide_benchmark.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
	if( argc != 5 )
	{
		std::cout << "Please provide sigma_s, sigma_r, inputFileName, outputFileName" << std::endl;
		return 1;
	}
    float sigma_s = std::stof(argv[1]);
    float sigma_r = std::stof(argv[2]);
    int timing_iterations = 15;
    Buffer<uint8_t> input = load_and_convert_image(argv[3]);
    std::cout << "Image imported from " << argv[3] << std::endl;
    Buffer<uint8_t> output(input.width(), input.height());

    // Manually-tuned version
    double min_t_manual = benchmark(timing_iterations, 10, [&]() {
        bilateral_filter_autoschedule_false_generated(input, sigma_r, sigma_s, output);
        output.device_sync();
    });
    printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);

    // Auto-scheduled version
	// Ben [9/06/20] for some reason the autoscheduled version has an improper access to the output image and fails
    /*double min_t_auto = benchmark(timing_iterations, 10, [&]() {
	bilateral_filter_autoschedule_true_generated(input, r_sigma, output);
        output.device_sync();
    });
    printf("Auto-scheduled time: %gms\n", min_t_auto * 1e3);*/

    convert_and_save_image(output, argv[4]);

    return 0;
}
