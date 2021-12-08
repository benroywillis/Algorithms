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
    Input<double> ku{"ku"};
	// parameter for something
    Input<double> R0{"R0"};
	// parameter for something
    Input<double> dR{"dR"};
	// parameter for something
    Input<double> dxdy{"dxdy"};
	// parameter for something
    Input<double> z0{"z0"};
	// output is x, y, c backprojected image
    Output<Buffer<float>> image{"image", 3};
	
    void generate() {
		// induction variables over radarPixels_x, radarPixels_y, pulse, complex
		Var x("x"), y("y"), p("p"), c("c");
		// clamps induction variables to input array bounds
		ComplexFunc in_data("in_data");
		in_data(x, y) = ComplexExpr( data(0, y, x), data(1, y, x) );

		// compute diffs between the current coordinates and the camera position
		Func xdiff("xdiff");
		xdiff(x, p) = platpos(0, p) - (-BP_NPIX_X / Halide::cast<double>(2.0f) + Halide::cast<double>(0.5f) + Halide::cast<double>(x)) * dxdy;
		Func ydiff("ydiff");
		ydiff(y, p) = platpos(1, p) - (-BP_NPIX_Y / Halide::cast<double>(2.0f) + Halide::cast<double>(0.5f) + Halide::cast<float>(y)) * dxdy;
		Func zdiff("zdiff");
		zdiff(p) = platpos(2, p)  - z0;
		Func R("R");
		R(x, y, p) = Halide::sqrt( xdiff(x, p)*xdiff(x, p) + ydiff(y, p)*ydiff(y, p) + zdiff(p)*zdiff(p) );
		Func bin("bin");
		bin(x, y, p) = ( R(x, y, p) - R0 ) * Halide::cast<double>(1.0f) / dR;

		// compute current sample weight, if bin is within the correct range
		Func compute_weight("compute_weight");
		compute_weight(x, y, p) = Halide::cast<float>(bin(x, y, p) - Halide::floor(bin(x, y, p)));
		// compute current sample
		ComplexFunc compute_sample("compute_sample");
		compute_sample(x, y, p) = ComplexExpr( 
			(1.0f-compute_weight(x, y, p)) * in_data(p, clamp(cast<int>(Halide::floor(bin(x, y, p))), 0, data.height()-1)).re()+
			 compute_weight(x, y, p)*in_data(p, clamp(cast<int>(Halide::floor(bin(x, y, p))), 0, data.height()-1)).re(), 
			(1.0f-compute_weight(x, y, p)) * in_data(p, clamp(cast<int>(Halide::floor(bin(x, y, p))+1), 0, data.height()-1)).im()+
			compute_weight(x, y, p)*in_data(p, clamp(cast<int>(Halide::floor(bin(x, y, p))+1), 0, data.height()-1)).im() );
		// compute matched filter weights
		ComplexFunc matched_filter("matched_filter");
		matched_filter(x, y, p) = ComplexExpr( Halide::cast<float>(Halide::cos( Halide::cast<double>(2.0f) * R(x, y, p) * ku )),
											   Halide::cast<float>(Halide::sin( Halide::cast<double>(2.0f) * R(x, y, p) * ku )) );
		// multiply sample by matched filter
		ComplexFunc mul_sample_filter("mul_sample_filter");
		mul_sample_filter(x, y, p) = compute_sample(x, y, p)*matched_filter(x, y, p);
//(0 <= bin(x, y, p)) && (bin(x, y, p) < N_RANGE_UPSAMPLED-2)
		// Function determines if we accumulate the current pulse or not (bin must fall within the correct range)
		ComplexFunc accum("accum");
		accum(x, y) = ComplexExpr();
		//Func valid_bin("valid_bin");
		//valid_bin(x, y, p) = ;
		//valid_bin.compute_root();
		RDom r(0, N_PULSES);
		accum(x, y) +=  ComplexExpr(Halide::tuple_select(
			(((Halide::cast<double>(0) <= bin(x, y, r)) && (bin(x, y, r) < (N_RANGE_UPSAMPLED-1))) ), 
			 mul_sample_filter(x, y, r), 
			ComplexExpr() ));
		// write output
		image(c, y, x) = 0.0f;
		/*image(0, y, x) = accum(x, y).re();
		image(1, y, x) = accum(x, y).im();*/
		/*image(0, y, x) = Halide::cast<float>(((Halide::cast<double>(0) <= bin(x, y, r)) && (bin(x, y, r) < (N_RANGE_UPSAMPLED-1))));
		image(1, y, x) = Halide::cast<float>(bin(x, y, r));*/
		/*image(0, y, x) = accum(x, y).re(); 
		image(1, y, x) = accum(x, y).im();*/
		RDom o(0, N_PULSES);
		//image(0, y, x) = Halide::cast<float>(bin(x, y, o));
		image(0, y, x) = Halide::cast<float>(ydiff(x,o));
		image.trace_stores();
	}
};

}  // namespace

HALIDE_REGISTER_GENERATOR(BP, bp)
