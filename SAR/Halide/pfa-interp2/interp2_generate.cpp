#include "Halide.h"
#include <math.h>

#include "sar_utils.h"
#include "sar_interp2.h"

#define M_PIf	(float)M_PI

namespace {

class Interpolation_2 : public Halide::Generator<Interpolation_2> {
public:
	// pulses, range, complex
    Input<Buffer<double>> data{"data", 3};
	// PFA
    Input<Buffer<float>> window{"window", 1};
	// range, pulses
    Input<Buffer<double>> input_coords{"input_coords", 2};
	// range
    Input<Buffer<double>> output_coords{"output_coords", 1};
	// azimuth, range, complex
    Output<Buffer<double>> resampled{"resampled", 3};
	
    void generate() {
		// induction variables over pulse, range, pfa, azimuth, complex
		Var p("p"), r("r"), pfa("pfa"), a("a"), c("c");
		// passthrough for debugging
		resampled = data;
	}
};

}  // namespace

HALIDE_REGISTER_GENERATOR(Interpolation_2, interp2)
