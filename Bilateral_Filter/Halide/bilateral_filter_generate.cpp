#include "Halide.h"
#include "halide_trace_config.h"
#include "halide_image_io.h"
#include <math.h>

#define M_PIf	(float)M_PI

namespace {

class BilateralFilter : public Halide::Generator<BilateralFilter> {
public:
    Input<Buffer<uint8_t>> input{"input", 3};
    Input<float> sigma_r{"sigma_r"};
    Input<float> sigma_s{"sigma_s"};

    Output<Buffer<uint8_t>> bilateral_filter{"bilateral_filter", 3};

    void generate() {
    	Var x("x"), y("y"), c("c");
    	Var x0("x"), y0("y");
    	Var x1("x"), y1("y");

        // Add a boundary condition
		// this returns the nearest sample every time we go beyond a sample boundary
        Func clamped = Halide::BoundaryConditions::repeat_edge(input);

		// Norm functions for spacial and rangeelements
		Func i_2;
		Func c_2;
		// Constants in Halide must be floats, therefore we have to declare our exponents as floats
		i_2(x) = pow( cast<double>(x), cast<double>(2.0f) );
		c_2(x) = pow( cast<double>(x), cast<double>(2.0f) );
		Func norm_space;
		norm_space(x0, y0, x1, y1) = pow( i_2(x1-x0) + i_2(y1 - y0) + i_2(y), cast<double>(0.5f) );
		
		// Norm function for "range" elements (color space)
		Func gray;
		gray(x, y) = cast<double>(clamped(x, y, 0)) / 3.0f + cast<double>(clamped(x, y, 1)) / 3.0f + cast<double>(clamped(x, y, 2)) / 3.0f;
		Func norm_range;
		norm_range(x0, y0, x1, y1) = abs( gray(x1, y1) - gray(x0, y0) );

        // Gaussian functions
		Func f;
		f( x0, y0, x1, y1 ) = ( 1.0f / pow( 2.0f * sigma_s * M_PIf, 0.5f ) )*exp( (-1.0f/2.0f) * (norm_space(x0, y0, x1, y1)*norm_space(x0, y0, x1, y1) / 2.0f*sigma_s) );
		Func g;
		g( x0, y0, x1, y1 ) = ( 1.0f / pow( 2.0f * sigma_r * M_PIf, 0.5f ) )*exp( (-1.0f/2.0f) * (norm_range(x0, y0, x1, y1)*norm_range(x0, y0, x1, y1) / 2.0f*sigma_r) );
		
		// sliding window
		Func Wp_x;
		Func Wp_y;
		Wp_x(x, y) = f(x, y, x, y)*g(x, y, x, y) + f(x, y, x+1, y)*g(x, y, x+1, y) + f(x, y, x+2, y)*g(x, y, x+2, y) + f(x, y, x-1, y)*g(x, y, x-1, y) + f(x, y, x-2, y)*g(x, y, x-2, y);
		Wp_y(x, y) = Wp_x(x, y, x, y) + Wp_x(x, y, x, y+1) + Wp_x(x, y, x, y+2) + Wp_x(x, y, x, y-1) + Wp_x(x, y, x, y-2);

		Func blurx;
		Func blury;
		blurx(x, y) = f(x, y, x, y)*g(x, y, x, y)*gray(x, y) + f(x, y, x+1, y)*g(x, y, x+1, y)*gray(x+1, y) + f(x, y, x+2, y)*g(x, y, x+2, y)*gray(x+2, y) + f(x, y, x-1, y)*g(x, y, x-1, y)*gray(x-1, y) + f(x, y, x-2, y)*g(x, y, x-2, y)*gray(x-2, y);
		blury(x, y) = blurx(x, y) + blurx(x, y+1) + blurx(x, y+2) + blurx(x, y-1) + blurx(x, y-2);

		Func output("output");
		output(x, y) = blury(x, y) / Wp_y(x, y);
		output(x, y) = cast<uint8_t>(output(x, y));
		//bilateral_filter = output.realize(input.width(), input.height(), 1);
		//Halide::Tools::save_image(bilateral_filter, "filtered");
	
		// estimates for the input image dimensions
		//input.set_estimates({{0, 512}, {0, 512}});
	
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(BilateralFilter, bilateral_filter)
