#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
//#include "bilateral_filter.h"
//#ifndef NO_AUTO_SCHEDULE
#include "bilateral_filter_autoschedule_true_generated.h"
#include "bilateral_filter_autoschedule_false_generated.h"
//#endif

#include "HalideBuffer.h"
#include "halide_benchmark.h"
#include "halide_image_io.h"

#define PROGPATH DASH_DATA "Halide/benchmarks/"
#define INPUT0 PROGPATH "mona_lisa.png"
#define OUTPUT0 PROGPATH "bilateral_filter_output.png"

using namespace Halide::Tools;
using namespace Halide::Runtime;

int main(int argc, char **argv) {
    float r_sigma = 0.1;
    float s_sigma = 0.1;
    int timing_iterations = 10;
    Buffer<float> input = load_and_convert_image(INPUT0);
    std::cout << "Image imported from " << INPUT0 << std::endl;
    Buffer<float> output(input.width(), input.height());

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

    convert_and_save_image(output, OUTPUT0);

    return 0;
}
