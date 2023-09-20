// This FFT is an implementation of the algorithm described in
// http://research.microsoft.com/pubs/131400/fftgpusc08.pdf
// This algorithm is more well suited to Halide than in-place
// algorithms.

#include "Halide.h"
#include <cmath>  // for log2
#include <cstdio>
#include <vector>

#include "fft.h"
#include "halide_benchmark.h"

#ifdef WITH_FFTW
#include <fftw3.h>
#endif

#include "fft_autoschedule_false_generated.h"

using namespace Halide;
using namespace Halide::Tools;
using namespace Halide::Runtime;

Var x("x"), y("y");



int main(int argc, char **argv) 
{
    // Set defaults
    int W = 512; //dimension W
    int H = 0; //dimension H
    int D = -1 //direction {-1:forward, 1:backward}
    
    // Parse args
    if (argc >= 4) {
        W = atoi(argv[1]);
        H = atoi(argv[2]);
	D = atoi(argv[3]);
    }

    // Declare buffers
    if(direction == -1)
    	Buffer<float> gain((1.0f/(W*H)));
    else
    	Buffer<float> gain((1.0f));
    Buffer<int32_t> vector_width(0);
    Buffer<bool> parallel(true);
    Buffer<int> direction(D);
    Buffer<bool> input_number_type(false); //{true:real, false:complex}
    Buffer<bool> output_number_type(false); //{true:real, false:complex}
    Buffer<int32_t> size0(W);
    Buffer<int32_t> size1(H);
    Buffer<float> input(W, H);
    Buffer<float> output(W, H);

    // Generate input
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            in(x, y) = (float)rand() / (float)RAND_MAX;
        }
    }

    // Measure performance
    int timing_iterations = 15;
    double min_t_manual = benchmark(timing_iterations, 10, [&]() {
        fft_autoschedule_false_generated(
					 gain, 
					 vector_width, 
					 parallel,
					 direction,
					 input_number_type,
					 output_number_type,
					 size0,
					 size1,
					 input,
					 output);
        output.device_sync();
    });
    printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);

    return 0;
}
