#include "Halide.h"
#include <math.h>

#include "sar_utils.h"

#define M_PIf	(float)M_PI

namespace {

class BP : public Halide::Generator<BP> {
public:
	// input is pulses, upsampled, complex
    Input<Buffer<double>> data{"data", 3};
	// parameter for something
    Input<Buffer<double>> platpos{"platpos", 1};
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
    Output<Buffer<double>> image{"image", 3};
	

    void generate() {
   /* 	Var x("x"), y("y"), f("f");
		// john: if you make these variables the same type as the two above, Halide will likely have a problem with it
    	Var a("a"), b("b");
    	Var d("d"), e("e");

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
	*/
		// induction variables over x, y, pulse, complex
		Var x("x"), y("y"), p("p"), c("c");
		// clamps induction variables to input array bounds
		Func input("input");
		input(x, y, c) = data( clamp(x, 0, data.height()), clamp(y, 0, data.width()-1), clamp(c, 0, 2) );
		// passthrough for debugging
		image(x, y, c) = input(x, y, c);
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(BP, bp)
