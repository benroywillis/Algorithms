#include "Halide.h"
#include <iostream>

namespace {

class ImageRegistration : public Halide::Generator<ImageRegistration> {
private:
	// jacobian matrix buffer
public:
	// input is 2 floating point first-derivatives of the image along both axes (x or y)
    Input<Buffer<float>> Dx{"Dx", 2};
    Input<Buffer<float>> Dy{"Dy", 2};
    Input<Buffer<float>> jacobian{"jacobian", 3};
	// output is a 6x6 floating point hessian matrix
    Output<Buffer<float>> LucasKanade{"LucasKanade", 2};
	// params define the compression, distortion and translation of the image warp
	Expr h_comp = (float)0.965;
	Expr h_dist = (float)0.01;
	Expr h_tran = (float)5.5;
	Expr v_comp = (float)0.965;
	Expr v_dist = (float)(-0.01);
	Expr v_tran = (float)5.5;

    void generate() {
		// induction over the input image
		Var x("x"), y("y");
		// induction over hessian
		Var k("k"), l("l");
		
		// first, warp the image
		// this is done using linear interpolation
		// the first step is to calculate indices for the input image using the compression, distortion and translation parameters from the input
		Func localx("localx");
		localx(x, y) = clamp( cast<float>(y)*h_comp + cast<float>(x)*v_dist + h_tran, (float)0.0, cast<float>(Dx.height()) );
		Func localy("localy");
		localy(x, y) = clamp( cast<float>(y)*h_dist + cast<float>(x)*v_comp + v_tran, (float)0.0, cast<float>(Dx.width()) );
		// next, convert these floating point numbers to array indices by taking their floor
		Func x_Index("x_Index");
		x_Index(x, y) = cast<int>(localx(x, y));
		Func y_Index("y_Index");
		y_Index(x, y) = cast<int>(localy(x, y));
		// next, find out how much we just truncated
		Func xCom("xCom");
		xCom(x, y) = localx(x, y) - cast<int>(localx(x, y));
		Func yCom("yCom");
		yCom(x, y) = localx(x, y) - cast<int>(localy(x, y));
		// calculate the inverse of the truncation
		Func xComi("xComi");
		xComi(x, y) = 1 - xCom(x, y);
		Func yComi("yComi");
		yComi(x, y) = 1 - yCom(x, y);
		// Now interpolate using the indices from above and the truncation errors as weights
		Func warp_x("warp_x");
		warp_x(x, y) = Dx( x_Index(x, y), y_Index(x, y) )*xCom(x, y)*yCom(x, y) +
							Dx( x_Index(x, y)+1, y_Index(x, y) )*xCom(x, y)*yComi(x, y) +
							Dx( x_Index(x, y), y_Index(x, y)+1 )*xComi(x, y)*yCom(x, y) +
							Dx( x_Index(x, y)+1, y_Index(x, y)+1 )*xComi(x, y)*yComi(x, y);
		Func warp_y("warp");
		warp_y(x, y) = Dy( x_Index(x, y), y_Index(x, y) )*xCom(x, y)*yCom(x, y) +
							Dy( x_Index(x, y)+1, y_Index(x, y) )*xCom(x, y)*yComi(x, y) +
							Dy( x_Index(x, y), y_Index(x, y)+1 )*xComi(x, y)*yCom(x, y) +
							Dy( x_Index(x, y)+1, y_Index(x, y)+1 )*xComi(x, y)*yComi(x, y);

		// second, calculate jacobian matrix
		// this is used to find the "steepest descent" of the image
		jacobian(x, y, 0) = cast<float>(x)*warp_x(x, y);
		jacobian(x, y, 1) = cast<float>(y)*warp_y(x, y);
		jacobian(x, y, 2) = cast<float>(y)*warp_x(x, y);
		jacobian(x, y, 3) = cast<float>(x)*warp_y(x, y);
		jacobian(x, y, 4) = warp_x(x, y);
		jacobian(x, y, 5) = warp_y(x, y);

		// third, calculate hessian
		// for each entry in the hessian, we accumulate the correlation of all permutations of pairs of rows over a 6-row window
		Func clamped_J("clamped_j");
		clamped_J(x, y, k) = jacobian( clamp(x, 0, Dx.height()-6), clamp(y, 0, Dx.width()), k ); 
		RDom c(0, Dx.width());
		Func crosscorr_rows("crosscorr_rows");
		crosscorr_rows(x, k, l) += clamped_J( x, c.x, k ) * clamped_J( x, c.x, l );
		// each index of the hessian is a reduction over the column space of the window
		RDom r(0, Dx.height(), 0, 6, 0, 6);
		Func hessian("hessian");
		hessian(r.y, r.z) += crosscorr_rows(r.x, r.y, r.z);
		LucasKanade = hessian;
		// john: stop early and dump a reference image. This will compartmentalize the good code and bad code
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ImageRegistration, LucasKanade)
