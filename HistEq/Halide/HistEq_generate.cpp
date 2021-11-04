#include "Halide.h"
#include "halide_trace_config.h"
#include "halide_image_io.h"
#include <math.h>
#include <float.h>

#define M_PIf	(float)M_PI

namespace {

class HistogramEqualization : public Halide::Generator<HistogramEqualization> {
public:
	// input image is grayscale 640x480
    Input<Buffer<uint8_t>> input{"input", 2};
	Input<uint8_t> num_bins{"num_bins", 1};
    Output<Buffer<uint8_t>> HistEq{"HistEq", 2};

    void generate() {
		// induction over the input image
		Var x("x"), y("y");
		// induction over the saturation values
		Var sat("sat");
		// histogram
		Func hist("hist");
		// histogram's reduction domain over the input
		RDom r(input);
		hist( sat ) = 0;
		hist( input(r.x, r.y) ) += 1;
		// this says that hist must go over its entire domain before g can occur
		hist.compute_root();

		// compute CDF
		// the reduction domain is over all possible intensity values ie the space formed by Var sat
		RDom b(1, 255);
		Expr cdf_rolling = cast<float>(hist(0));
		Func cdf("cdf");
		cdf(sat) = cdf_rolling;
		cdf_rolling += cast<float>(hist(b));
		cdf.compute_root();

		// find min distribution weight
		Expr min_cdf = FLT_MAX;
		min_cdf = Halide::Internal::Min::make( cdf(sat), min_cdf );

		// LUT maps a pixel sat to its output saturation value
		Func lut("lut");
		lut(sat) = cast<uint8_t>( cast<float>( cdf(sat) - min_cdf ) * cast<float>( num_bins - 1 ) / cast<float>(num_bins - min_cdf) );

		// Map input pixels to their transformed saturation level
		Func output("output");
		output(x, y) = lut( input(x, y) );

		// map the last pipe stage to the output
		HistEq = output;
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(HistogramEqualization, HistEq)
