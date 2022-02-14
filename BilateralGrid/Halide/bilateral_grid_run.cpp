#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "bilateral_grid_autoschedule_false_generated.h"

#include "HalideBuffer.h"
#include "halide_benchmark.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

template<typename T>
void gray(Buffer<T>& image, Buffer<T>& out)
{
    //Buffer<T> out(image.width(), image.height());  //the 3rd param is needed to output png.
	for( int j = 0; j < image.width(); j++ )
	{
		for( int i = 0; i < image.height(); i++ )
		{
    		out(j, i) = 0.299f * image(j, i, 0) + 0.587f * image(j, i, 1) + 0.114f * image(j, i, 2);
		}
	}
	//return out;
}

int main(int argc, char **argv) {
    if (argc < 5) {
        printf("Usage: ./filter input.png output.png range_sigma space_sigma\n"
               "e.g. ./filter input.png output.png 0.1 0.1\n");
        return 0;
    }

    float r_sigma = (float)atof(argv[3]);
    int s_sigma = atoi(argv[4]);
    int timing_iterations = 15;

    Buffer<float> input = load_and_convert_image(argv[1]);
    Buffer<float> grayscale(input.width(), input.height());
    Buffer<float> output(input.width(), input.height());

	gray<float>(input, grayscale);
	//std::cout << "grayscale image access: " << grayscale(input.width(),input.height()) << std::endl;

    // Timing code. Timing doesn't include copying the input data to
    // the gpu or copying the output back.
	std::cout << "Running halide benchmark!" << std::endl;
    // Manually-tuned version
    double min_t_manual = benchmark(timing_iterations, 10, [&]() {
        bilateral_grid_autoschedule_false_generated(grayscale, r_sigma, s_sigma, output);
        output.device_sync();
    });
    printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);

    printf("Success!\n");
    return 0;
}
