#include <cassert>
#include <cstdio>
#include <cstdlib>

#include "HalideBuffer.h"
#include "HalideRuntime.h"

#include "iir_blur_autoschedule_false_generated.h"
//#include "iir_blur_autoschedule_true_generated.h"

#include "halide_benchmark.h"
#include "halide_image_io.h"

#include "TimingLib.h"

using namespace Halide::Tools;

int main(int argc, char **argv) {
    if (argc != 5) {
        printf("Usage: %s alpha input_image output_image threads\n", argv[0]);
        return 1;
    }
	float alpha = strtof(argv[1], NULL);

	int threads = atoi(argv[4]);
	printf("Setting thread count to %d\n", threads);
	halide_set_num_threads(threads);

    Halide::Runtime::Buffer<float> input = load_and_convert_image(argv[2]);
    Halide::Runtime::Buffer<float> output(input.width(), input.height(), input.channels());

    double best_manual = __TIMINGLIB_benchmark( [&](){ iir_blur_autoschedule_false_generated(input, alpha, output); output.device_sync(); } );
	// the autoschedule takes about twice as much time as the default halide schedule
    //best_manual = __TIMINGLIB_benchmark( [&](){ iir_blur_autoschedule_true_generated(input, alpha, output); output.device_sync(); } );

	/*
    double best_auto = benchmark([&]() {
        iir_blur_auto_schedule_true_generated(input, 0.5f, output);
        output.device_sync();
    });
    printf("Auto-scheduled time: %gs\n", best_auto);
	*/

    convert_and_save_image(output, argv[3]);

    printf("Success!\n");
    return 0;
}
