#include "Halide.h"
#include <math.h>

#include "sar_utils.h"
#include "sar_interp1.h"

#define M_PIf	(float)M_PI

namespace {

class Interpolation_1 : public Halide::Generator<Interpolation_1> {
public:
	// pulses, range, complex
    Input<Buffer<double>> data{"data", 3};
	// PFA
    Input<Buffer<float>> window{"window", 1};
	// pulses
    Input<Buffer<double>> input_start_coords{"input_start_coords", 1};
	// pulses
    Input<Buffer<double>> input_coord_spacing{"input_coord_spacing", 1};
	// pfa_nout_range
    Input<Buffer<double>> output_coords{"output_coords", 1};
	// pulses, range, complex
    Output<Buffer<double>> imsampled{"resampled", 3};
	
    void generate() {
		// induction variables over pulses, range, PFA, pfa_nout_range
		Var p("p"), r("r"), pfa("pfa"), pnr("pnr");
		// passthrough for debugging
		imsampled = data;
	}
};

}  // namespace

HALIDE_REGISTER_GENERATOR(Interpolation_1, interp1)
