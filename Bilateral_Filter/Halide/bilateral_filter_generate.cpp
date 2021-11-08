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

    Output<Buffer<uint8_t>> bilateral_filter{"bilateral_filter", 2};

    void generate() {
    	Var x("x"), y("y"), c("c");
		// john: if you make these variables the same type as the two above, Halide will likely have a problem with it
    	Var a("a"), b("b");
    	Var d("d"), e("e");
		// boundary condition for when we have to select pixels for the window that are outside the image boundaries
		Func clamped = Halide::BoundaryConditions::repeat_edge(input);

		// Norm functions for spacial and range elements
		Func i_2;
		Func c_2;
		// Constants in Halide must be floats, therefore we have to declare our exponents as floats
		i_2(x) = pow( cast<double>(x), cast<double>(2.0f) );
		c_2(x) = pow( cast<double>(x), cast<double>(2.0f) );
		Func norm_space;
		norm_space(a, b, d, e) = pow( i_2(d-a) + i_2(e - b), cast<double>(0.5f) );

		// Norm function for "range" elements (color space)
		Func gray;
		gray(x, y) = cast<double>(clamped(x, y, 0)) / 3.0f + cast<double>(clamped(x, y, 1)) / 3.0f + cast<double>(clamped(x, y, 2)) / 3.0f;
		Func norm_range;
		norm_range(a, b, d, e) = abs( gray(d, e) - gray(a, b) );

        // Gaussian functions
		Func f;
		f( a, b, d, e ) = ( 1.0f / pow( 2.0f * sigma_s * M_PIf, 0.5f ) )*exp( (-1.0f/2.0f) * (norm_space(a, b, d, e)*norm_space(a, b, d, e) / 2.0f*sigma_s) );
		Func g;
		g( a, b, d, e ) = ( 1.0f / pow( 2.0f * sigma_r * M_PIf, 0.5f ) )*exp( (-1.0f/2.0f) * (norm_range(a, b, d, e)*norm_range(a, b, d, e) / 2.0f*sigma_r) );
	
		// normalizer for each pixel blur
		Func Wp_x;
		Func Wp_y;
		Wp_x(x, y) = f(x, y, x, y)*g(x, y, x, y) + f(x, y, x+1, y)*g(x, y, x+1, y) + f(x, y, x+2, y)*g(x, y, x+2, y) + f(x, y, x-1, y)*g(x, y, x-1, y) + f(x, y, x-2, y)*g(x, y, x-2, y);
		Wp_y(x, y) = Wp_x(x, y) + Wp_x(x, y+1) + Wp_x(x, y+2) + Wp_x(x, y-1) + Wp_x(x, y-2);

		// each pixel blur
		Func blurx;
		Func blury;
		blurx(x, y) = f(x, y, x, y)*g(x, y, x, y)*gray(x, y) + f(x, y, x+1, y)*g(x, y, x+1, y)*gray(x+1, y) + f(x, y, x+2, y)*g(x, y, x+2, y)*gray(x+2, y) + f(x, y, x-1, y)*g(x, y, x-1, y)*gray(x-1, y) + f(x, y, x-2, y)*g(x, y, x-2, y)*gray(x-2, y);
		blury(x, y) = blurx(x, y) + blurx(x, y+1) + blurx(x, y+2) + blurx(x, y-1) + blurx(x, y-2);

		Func output("output");
		output(x, y) = cast<uint8_t>(clamp( blury(x, y) / Wp_y(x, y) , 0.0f, 255.0f ));
		bilateral_filter = output;
	
		// estimates for the input image dimensions
		//input.set_estimates({{0, 512}, {0, 512}});
	
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(BilateralFilter, bilateral_filter)
