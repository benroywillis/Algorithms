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
    Input<Buffer<int>> input{"input", 2};
	Input<int> num_bins{"num_bins"};
    Output<Buffer<int>> HistEq{"HistEq", 2};

    void generate() {
		// induction over the input image
		Var x("x"), y("y");
		// induction over the saturation values
		Var sat("sat");
		// function for reading in the pixels in the correct order
		Func in("in");
		in(x, y) = clamp( input(y, x), 0, num_bins-1 );
		// histogram
		Func hist("hist");
		// histogram's reduction domain over the input
		RDom r(input);
		hist( sat ) = 0;
		hist( in(r.y, r.x) ) += 1;
		// this says that hist must go over its entire domain before cdf can occur
		hist.compute_root();

		// compute CDF
		// the reduction domain is over all possible intensity values ie the space formed by Var sat
		// find min distribution weight
		Func cdf("cdf");
		cdf(sat) = Halide::cast<double>(0);
		RDom s(input);
		cdf(in(s.y, s.x)) += cast<double>(hist( in(s.y, s.x) ));
		cdf.compute_root();
		RDom t(input);
		Expr min_cdf = Halide::cast<double>(FLT_MAX);
		min_cdf = cast<double>(Halide::Internal::Min::make( cdf(in(t.y, t.x)), min_cdf ));

		// LUT maps a pixel sat to its output saturation value
		Func lut("lut");
		lut(sat) = cast<double>(0.0f);
		//lut(sat) = cast<double>( cdf(sat) - min_cdf(sat) ) * cast<double>( num_bins - 1 ) / cast<double>(cast<double>(input.width()*input.height()) - min_cdf(sat));
		lut.compute_root();

		// Map input pixels to their transformed saturation level
		//HistEq(y, x) = cast<int>(lut(in( x,y )));
		HistEq(y, x) = 0;
		HistEq(y, x) = cast<int>(min_cdf);
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(HistogramEqualization, HistEq)
