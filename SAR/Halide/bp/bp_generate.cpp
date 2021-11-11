#include "Halide.h"
#include <math.h>
#include <iostream>
#include "sar_utils.h"
#include "sar_backprojection.h"
#include "include/complex.h"

#define M_PIf	(float)M_PI

namespace {

class BP : public Halide::Generator<BP> {
public:
	// input is pulses, upsampled, complex
    Input<Buffer<float>> data{"data", 3};
	// parameter for something
    Input<Buffer<double>> platpos{"platpos", 2};
	// parameter for something
    Input<Buffer<double>> ku{"ku", 1};
	// parameter for something
    Input<Buffer<double>> R0{"R0", 1};
	// parameter for something
    Input<Buffer<double>> dR{"dR", 1};
	// parameter for something
    Input<Buffer<double>> dxdy{"dxdy", 1};
	// parameter for something
    Input<Buffer<double>> z0{"z0", 1};
	// output is x, y, c backprojected image
    Output<Buffer<float>> image{"image", 3};
	
    void generate() {
		// induction variables over radarPixels_x, radarPixels_y, pulse, complex
		Var x("x"), y("y"), p("p"), c("c");
		// clamps induction variables to input array bounds
		ComplexFunc in_data("in_data");
		in_data(x, y) = ComplexExpr( data(x, y, 0), data(x, y, 1) );

		// compute diffs between the current coordinates and the camera position
		Func xdiff("xdiff");
		xdiff(x, p) = platpos(p, 0) - (-BP_NPIX_X / Halide::cast<double>(2.0f) + Halide::cast<double>(0.5f) - Halide::cast<double>(x)) * dxdy(0);
		Func ydiff("ydiff");
		ydiff(y, p) = platpos(p, 1) - (-BP_NPIX_Y / Halide::cast<double>(2.0f) + Halide::cast<double>(0.5f) - Halide::cast<float>(y)) * dxdy(0);
		Func zdiff("zdiff");
		zdiff(p) = platpos(p, 2) - (-BP_NPIX_Y / Halide::cast<double>(2.0f) + Halide::cast<double>(0.5f) - z0(0)) * dxdy(0);
		Func R("R");
		R(x, y, p) = sqrt( xdiff(x, p)*xdiff(x, p) + ydiff(y, p)*ydiff(y, p) + zdiff(p)*zdiff(p) );
		Func bin("bin");
		bin(x, y, p) = ( R(x, y, p) - R0(0) ) * Halide::cast<double>(1.0f) / dR(0);

		// compute current sample weight, if bin is within the correct range
		Func compute_weight("compute_weight");
		compute_weight(x, y, p) = bin(x, y, p) - cast<int>(Halide::floor(bin(x, y, p)));
		// compute current sample
		ComplexFunc compute_sample("compute_sample");
		compute_sample(x, y, p).x = (Halide::cast<double>(1.0f)-compute_weight(x, y, p)) * in_data(p, cast<int>(Halide::floor(bin(x, y, p)))).re() + compute_weight(x, y, p)*in_data(p, cast<int>(Halide::floor(bin(x, y, p)))).re();
		compute_sample(x, y, p).y = (Halide::cast<double>(1.0f)-compute_weight(x, y, p)) * in_data(p, cast<int>(Halide::floor(bin(x, y, p)))).im() + compute_weight(x, y, p)*in_data(p, cast<int>(Halide::floor(bin(x, y, p)))).im();
		// compute matched filter weights
		ComplexFunc matched_filter("matched_filter");
		matched_filter(x, y, p).x = cos( Halide::cast<double>(2.0f) * R(x, y, p) * ku(0) );
		matched_filter(x, y, p).y = sin( Halide::cast<double>(2.0f) * R(x, y, p) * ku(0) );
		// multiply sample by matched filter
		ComplexFunc mul_sample_filter("mul_sample_filter");
		mul_sample_filter(x, y, p) = compute_sample(x, y)*matched_filter(x, y, p);
//(0 <= bin(x, y, p)) && (bin(x, y, p) < N_RANGE_UPSAMPLED-2)
		// Function determines if we accumulate the current pulse or not (bin must fall within the correct range)
		ComplexFunc accum("accum");
		accum(x, y) = ComplexExpr();
		RDom r(0, N_PULSES);
		accum(x, y) +=  ComplexExpr(Halide::tuple_select((0 <= bin(x, y, r)) && (bin(x, y, r) < N_RANGE_UPSAMPLED-2), mul_sample_filter(x, y, r), ComplexExpr() ));
		accum.compute_root();
		// write output
		image(x, y, c) = Halide::mux( c, {accum(x, y).re(), accum(x, y).im()} );
	}
};

}  // namespace

HALIDE_REGISTER_GENERATOR(BP, bp)
