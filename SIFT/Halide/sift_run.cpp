#include <cassert>
#include <cstdio>
#include <cstdlib>

#include "HalideBuffer.h"
#include "HalideRuntime.h"

#include "sift_autoschedule_false_generated.h"
//#include "sift_autoschedule_true_generated.h"

#include "halide_benchmark.h"
#include "halide_image_io.h"

#include "TimingLib.h"

using namespace Halide::Tools;

int main(int argc, char **argv) {
    if (argc != 8) {
        printf("Usage: %s octaves intervals curve_threshold contrast_threshold input_image output_image threads\n", argv[0]);
        return 1;
    }
	int octaves = atoi(argv[1]);
	int thresholds = atoi(argv[2]);
	float cur_thr= strtof(argv[3], NULL);
	float con_thr= strtof(argv[4], NULL);

	int threads = atoi(argv[7]);
	printf("Setting thread count to %d\n", threads);
	halide_set_num_threads(threads);

    Halide::Runtime::Buffer<float> input = load_and_convert_image(argv[5]);
    Halide::Runtime::Buffer<uint8_t> output(input.width(), input.height());

    double best_manual = __TIMINGLIB_benchmark( [&](){ sift_autoschedule_false_generated(input, cur_thr, con_thr, output); } );
	// the autoschedule takes about twice as much time as the default halide schedule
    //best_manual = __TIMINGLIB_benchmark( [&](){ sift_autoschedule_true_generated(input, alpha, output); output.device_sync(); } );

	/*
    double best_auto = benchmark([&]() {
        sift_auto_schedule_true_generated(input, 0.5f, output);
        output.device_sync();
    });
    printf("Auto-scheduled time: %gs\n", best_auto);
	*/

	unsigned points = 0;
	for( unsigned y = 0; y < input.height(); y++ )
	{
		for( unsigned x = 0; x < input.width(); x++ )
		{
			if( output(x, y) )
			{
				points++;
			}
		}
	}
	printf("Found %d keypoints\n", points);
	
    convert_and_save_image(output, argv[6]);

    printf("Success!\n");
    return 0;
}
