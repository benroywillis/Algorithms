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
		Func input("input");
		input(x, y, c) = data( clamp(x, 0, data.height()-1), clamp(y, 0, data.width()-1), clamp(c, 0, 2) );

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
		Func compute_sample_re("compute_sample_re");
		compute_sample_re(x, y, p) = (Halide::cast<double>(1.0f)-compute_weight(x, y, p)) * input(p, cast<int>(Halide::floor(bin(x, y, p))), 0) + compute_weight(x, y, p)*input(p, cast<int>(Halide::floor(bin(x, y, p))), 0);
		Func compute_sample_im("compute_sample_im");
		compute_sample_im(x, y, p) = (Halide::cast<double>(1.0f)-compute_weight(x, y, p)) * input(p, cast<int>(Halide::floor(bin(x, y, p))), 1) + compute_weight(x, y, p)*input(p, cast<int>(Halide::floor(bin(x, y, p))), 1);
		// compute matched filter weights
		Func matched_filter_re("matched_filter_re");
		matched_filter_re(x, y, p) = cos( Halide::cast<double>(2.0f) * R(x, y, p) * ku(0) );
		Func matched_filter_im("matched_filter_im");
		matched_filter_im(x, y, p) = sin( Halide::cast<double>(2.0f) * R(x, y, p) * ku(0) );
		// multiply sample by matched filter
		Func mul_sample_filter_re("mul_sample_filter_re");
		mul_sample_filter_re(x, y, p) = compute_sample_re(x, y, p)*matched_filter_re(x, y, p) - compute_sample_im(x, y, p)*matched_filter_im(x, y, p);
		Func mul_sample_filter_im("mul_sample_filter_im");
		mul_sample_filter_im(x, y, p) = compute_sample_re(x, y, p)*matched_filter_im(x, y, p) + compute_sample_im(x, y, p)*matched_filter_re(x, y, p);
		mul_sample_filter_im.trace_stores();

		// Function determines if we accumulate the current pulse or not (bin must fall within the correct range)
		Func accum_re("accum_re");
		accum_re(x, y, p) = Halide::mux( (0 <= bin(x, y, p)) && (bin(x, y, p) < N_RANGE_UPSAMPLED-2), { 0.0f, cast<float>(mul_sample_filter_re(x, y, p)) } );
		Func accum_im("accum_im");
		accum_im(x, y, p) = Halide::mux( (0 <= bin(x, y, p)) && (bin(x, y, p) < N_RANGE_UPSAMPLED-2), { 0.0f, cast<float>(mul_sample_filter_im(x, y, p)) } );
		// reduction domain over the space of pulses
		RDom r(0, N_PULSES);
		// write output
		image(x, y, c) = 0.0f;
		image(x, y, c) = Halide::mux( c, {accum_re(x, y, r), accum_im(x, y, r)} );
	}
};

}  // namespace

HALIDE_REGISTER_GENERATOR(BP, bp)
