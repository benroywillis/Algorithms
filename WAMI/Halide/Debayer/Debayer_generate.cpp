#include "Halide.h"
#include <iostream>
#include <math.h>
#include "wami_debayer.h"

#define PIXEL_MAX 65535

namespace {

class PERFECTDebayer : public Halide::Generator<PERFECTDebayer> {
public:
    Input<Buffer<u16>> bayer{"bayer", 2};
    Output<Buffer<u16>> debayered{"debayered", 3};

    void generate() {
		// induction over the input image
		Var x("x"), y("y");
		// induction over color space
		Var c("c");
		// interpolation points
		Var pos("pos"), neg("neg");
		// clamps the iterators to the boundaries of the input arrays
		Func input("input");
		input(x, y) = bayer( clamp(x, 0, bayer.height()), clamp(y, 0, bayer.width()) ); 

		// This algorithm demosaic's a bayer pattern
		// It makes four access patterns: (PAD,PAD), (PAD,PAD+1), (PAD+1,PAD), (PAD+1,PAD+1)
		// Copy red pixels through directly
		Func copy_red("copy_red");
		copy_red(x, y, c) = mux( (x%2==0) && (y%2==0) && (c==0), {input(x, y), cast<u16>(0)} );
		// Copy green_TR pixels through directly
		Func copy_green_TR("copy_green_TR");
		copy_green_TR(x, y, c) = mux( ((x+1)%2==0) && (y%2==0) && (c==1), {input(x, y), cast<u16>(0)} );
		// Copy green_BL pixels through directly
		Func copy_green_BL("copy_green_BL");
		copy_green_BL(x, y, c) = mux( (x%2==0) && ((y+1)%2==0) && (c==1), {input(x, y), cast<u16>(0)} );
		// Copy blue pixels through directly
		Func copy_blue("copy_blue");
		copy_blue(x, y, c) = mux( ((x+1)%2==0) && ((y+1)%2==0) && (c==2), {input(x, y), cast<u16>(0)} );
	
		// interpolation functions takes in interpolation points and computes the true interpolation point
		Func compute_clamp("compute_clamp");
		compute_clamp(pos, neg) = mux( (pos<<1) > neg, { cast<u16>(clamp( (pos-neg) >> 3, 0, PIXEL_MAX)), cast<u16>(0) } );
		Func compute_clamp_fractional_neg("compute_clamp_fractional_neg");
		compute_clamp_fractional_neg(pos, neg) = mux( (pos<<1) > neg, { cast<u16>(clamp( (pos-neg) >> 4, 0, PIXEL_MAX)), cast<u16>(0) } );
	
		Func interp_g_rrr_g_bbb("interp_g_rrr_g_bbb");
		interp_g_rrr_g_bbb(x, y) = compute_clamp( 2*input(x-1,y) + 2*input(x,y-1) + 4*input(x,y) + 2*input(x,y+1) + 2*input(x+1,y), input(x,y+2) + input(x-2,y) + input(x,y-2) + input(x+2,y) );
		// interpolate green pixels at red pixels
		Func interp_gr("interp_gr");
		interp_gr(x, y, c) = mux( (x%2==0) && (y%2==0) && (c==1), {interp_g_rrr_g_bbb(x, y), cast<u16>(0)} );
		// interpolate green pixels at blue pixels
		Func interp_gb("interp_gb");
		interp_gb(x, y, c) = mux( ((x+1)%2==0) && ((y+1)%2==0) && (c==1), {interp_g_rrr_g_bbb(x, y), cast<u16>(0)} );

		Func interp_r_grb_b_gbr("interp_r_grb_b_gbr");
		interp_r_grb_b_gbr(x, y) = compute_clamp( ((input(x-2,y)+input(x+2,y)) >> 1) + 4*input(x,y-1) + 5*input(x,y) + 4*input(x,y+1), input(x-1,y-1) + input(x-1,y+1) + input(x,y-2) + input(x,y+2) + input(x+1,y-1) + input(x+1,y+1) );
		// interpolate red pixels at green pixels, red row, blue column
		Func interp_rg("interp_rg");
		interp_rg(x, y, c) = mux( ( x%2==0 && (y+1)%2==0 && c==0 ), {interp_r_grb_b_gbr(x, y), cast<u16>(0)} );
		// interpolage blue pixels at green pixels, blue row, red column
		Func interp_bg("interp_bg");
		interp_bg(x, y, c) = mux( ( (x+1)%2==0 && y%2==0 && c==2 ), {interp_r_grb_b_gbr(x, y), cast<u16>(0)} );

		Func interp_r_gbr_b_grb("interp_r_grb_b_grb");
		interp_r_gbr_b_grb(x, y) = compute_clamp( 4*input(x-1, y) + ((input(x,y-2) + input(x,y+2))>>1) + 5*input(x,y) + 4*input(x+1,y), input(x-2,y) + input(x-1,y-1) + input(x-1,y+1) + input(x+1,y-1) + input(x+1,y+1) + input(x+2,y));
		// interpolate red pixels at green pixels, blue row, red colun
		Func interp_rgbr("interp_rgbr");
		interp_rgbr(x, y, c) = mux( ( (x+1)%2==0 && y%2==0 && c==0 ), {interp_r_gbr_b_grb(x,y), cast<u16>(0)} );
		// interpolate blue pixels at green pixels, red row, blue column
		Func interp_b_grb("interp_b_grb");
		interp_b_grb(x, y, c) = mux( ( x%2==0 && (y+1)%2==0 && c==2 ), {interp_r_gbr_b_grb(x,y), cast<u16>(0)} );

		Func interp_r_bbb_b_rrr("interp_r_bbb_b_rrr");
		interp_r_bbb_b_rrr(x, y) = compute_clamp_fractional_neg( 2*input(x-1,y-1) + 2*input(x-1,y+1) + 6*input(x,y) + 2*input(x+1,y-1) + 2*input(x+1,y+1), 3*input(x-2,y) + 3*input(x,y-2) + 3*input(x,y+2) + 3*input(x+2,y) );
		// interpolate red pixels at blue pixels, blue row, blue column
		Func interp_r_bbb("interp_r_bbb");
		interp_r_bbb(x, y, c) = mux( ( (x+1)%2==0 && (y+1)%2==0 && c==0 ), {interp_r_bbb_b_rrr(x, y), cast<u16>(0)} );
		// interpolate blue pixels at red pixels, red row, red column
		Func interp_b_rrr("interp_b_rrr");
		interp_b_rrr(x, y, c) = mux( ( x%2==0 && y%2==0 && c==2 ), {interp_r_bbb_b_rrr(x, y), cast<u16>(0)} );

		// write output
		debayered(x, y, c) = copy_red(x, y, c) + copy_green_TR(x, y, c) + copy_green_BL(x, y, c) + copy_blue(x, y, c) + interp_gr(x, y, c) + interp_gb(x, y, c) + interp_rg(x, y, c) + interp_bg(x, y, c) + interp_rgbr(x, y, c) + interp_b_grb(x, y, c) + interp_r_bbb(x, y, c) + interp_b_rrr(x, y, c);
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(PERFECTDebayer, Debayer)
