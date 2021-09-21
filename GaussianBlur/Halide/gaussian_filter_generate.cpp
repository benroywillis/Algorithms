#include "Halide.h"
#include "halide_trace_config.h"
#include "halide_image_io.h"
#include <math.h>

#define M_PIf	(float)M_PI

namespace {

class GaussianFilter : public Halide::Generator<GaussianFilter> {
public:
    Input<Buffer<uint8_t>> input{"input", 3};
    Input<float> sigma{"sigma"};

    Output<Buffer<uint8_t>> gaussian_filter{"gaussian_filter", 2};

    void generate() {
    	Var x, y;
		Func kernel, bounded, blur_y, blur;
		kernel(x) = exp(-x*x/(2*sigma*sigma))/(sqrtf(2 * M_PI) * sigma);
		bounded = Halide::BoundaryConditions::repeat_edge(input);
		blur_y(x, y) = (kernel(0) * input(x, y) + kernel(1) * (bounded(x, y-1) + bounded(x, y+1)) + kernel(2) * (bounded(x, y-2) + bounded(x, y+2)));
		blur(x, y) = (kernel(0) * blur_y(x, y) + kernel(1) * (blur_y(x-1, y) + blur_y(x+1, y)) + kernel(2) * (blur_y(x-2, y) + blur_y(x+2, y)));
		gaussian_filter = blur;
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(GaussianFilter, gaussian_filter)
