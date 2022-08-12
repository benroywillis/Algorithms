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
    if (argc < 3) {
        printf("Usage: %s in out\n", argv[0]);
        return 1;
    }
	if( argc == 4 )
	{
		int threads = atoi(argv[3]);
		printf("Setting thread count to %d\n", threads);
		halide_set_num_threads(threads);
	}

    Halide::Runtime::Buffer<float> input = load_and_convert_image(argv[1]);
    Halide::Runtime::Buffer<float> output(input.width(), input.height(), input.channels());

    double best_manual = __TIMINGLIB_benchmark( [&](){ iir_blur_autoschedule_false_generated(input, 0.5f, output); } );

	/*
    double best_auto = benchmark([&]() {
        iir_blur_auto_schedule_true_generated(input, 0.5f, output);
        output.device_sync();
    });
    printf("Auto-scheduled time: %gs\n", best_auto);
	*/

    convert_and_save_image(output, argv[2]);

    printf("Success!\n");
    return 0;
}
