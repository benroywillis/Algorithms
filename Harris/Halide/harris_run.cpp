#include <cassert>
#include <cstdio>
#include <cstdlib>

#include "HalideBuffer.h"
#include "HalideRuntime.h"

#include "harris_autoschedule_false_generated.h"
//#include "harris_autoschedule_true_generated.h"

#include "halide_benchmark.h"
#include "halide_image_io.h"

#include "TimingLib.h"

using namespace Halide::Tools;

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("Usage: %s input_image output_image threads\n", argv[0]);
        return 1;
    }
	int threads = atoi(argv[3]);
	printf("Setting thread count to %d\n", threads);
	halide_set_num_threads(threads);

    Halide::Runtime::Buffer<float> input = load_and_convert_image(argv[1]);
    Halide::Runtime::Buffer<float> output(input.width()-6, input.height()-6);
	output.set_min(3, 3);

    double best_manual = __TIMINGLIB_benchmark( [&](){ harris_autoschedule_false_generated(input, output); output.device_sync(); } );
	// the autoschedule takes about twice as much time as the default halide schedule
    //best_manual = __TIMINGLIB_benchmark( [&](){ harris_autoschedule_true_generated(input, output); output.device_sync(); } );

    convert_and_save_image(output, argv[2]);

    printf("Success!\n");
    return 0;
}
