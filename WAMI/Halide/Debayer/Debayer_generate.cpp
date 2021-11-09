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
		// clamps the iterators to the boundaries of the input arrays when we accumulate over windows
		Func input("input");
		input = Halide::BoundaryConditions::repeat_edge(bayer); 

		// This algorithm demosaic's a bayer pattern
		// It makes four access patterns: (PAD,PAD), (PAD,PAD+1), (PAD+1,PAD), (PAD+1,PAD+1)
		// Copy red pixels through directly
		Func copy_red("copy_red");
		copy_red(x, y, c) = Halide::mux( (x%2==0) && (y%2==0) && (c==0), {cast<u16>(0), input(x, y)} );
		// Copy green_TR pixels through directly
		Func copy_green_TR("copy_green_TR");
		copy_green_TR(x, y, c) = Halide::mux( ((x+1)%2==0) && (y%2==0) && (c==1), {cast<u16>(0), input(x, y)} );
		// Copy green_BL pixels through directly
		Func copy_green_BL("copy_green_BL");
		copy_green_BL(x, y, c) = Halide::mux( (x%2==0) && ((y+1)%2==0) && (c==1), {cast<u16>(0), input(x, y)} );
		// Copy blue pixels through directly
		Func copy_blue("copy_blue");
		copy_blue(x, y, c) = Halide::mux( ((x+1)%2==0) && ((y+1)%2==0) && (c==2), {cast<u16>(0), input(x, y)} );
	
		// interpolation functions takes in interpolation points and computes the true interpolation point
		Func compute_clamp("compute_clamp");
		compute_clamp(pos, neg) = Halide::mux( (pos<<1) > neg, {cast<u16>(0),  cast<u16>(clamp( (pos-neg) >> 3, 0, PIXEL_MAX)) } );
		Func compute_clamp_fractional_neg("compute_clamp_fractional_neg");
		compute_clamp_fractional_neg(pos, neg) = Halide::mux( (pos<<1) > neg, {cast<u16>(0),  cast<u16>(clamp( (pos-neg) >> 4, 0, PIXEL_MAX)) } );
	
		Func interp_g_rrr_g_bbb("interp_g_rrr_g_bbb");
		interp_g_rrr_g_bbb(x, y) = compute_clamp( 2*input(x-1,y) + 2*input(x,y-1) + 4*input(x,y) + 2*input(x,y+1) + 2*input(x+1,y), input(x,y+2) + input(x-2,y) + input(x,y-2) + input(x+2,y) );
		// interpolate green pixels at red pixels
		Func interp_gr("interp_gr");
		interp_gr(x, y, c) = Halide::mux( (x%2==0) && (y%2==0) && (c==1), {cast<u16>(0), interp_g_rrr_g_bbb(x, y)} );
		// interpolate green pixels at blue pixels
		Func interp_gb("interp_gb");
		interp_gb(x, y, c) = Halide::mux( ((x+1)%2==0) && ((y+1)%2==0) && (c==1), {cast<u16>(0), interp_g_rrr_g_bbb(x, y)} );

		Func interp_r_grb_b_gbr("interp_r_grb_b_gbr");
		interp_r_grb_b_gbr(x, y) = compute_clamp( ((input(x-2,y)+input(x+2,y)) >> 1) + 4*input(x,y-1) + 5*input(x,y) + 4*input(x,y+1), input(x-1,y-1) + input(x-1,y+1) + input(x,y-2) + input(x,y+2) + input(x+1,y-1) + input(x+1,y+1) );
		// interpolate red pixels at green pixels, red row, blue column
		Func interp_rg("interp_rg");
		interp_rg(x, y, c) = Halide::mux( ( x%2==0 && (y+1)%2==0 && c==0 ), {cast<u16>(0), interp_r_grb_b_gbr(x, y)} );
		// interpolage blue pixels at green pixels, blue row, red column
		Func interp_bg("interp_bg");
		interp_bg(x, y, c) = Halide::mux( ( (x+1)%2==0 && y%2==0 && c==2 ), {cast<u16>(0), interp_r_grb_b_gbr(x, y)} );

		Func interp_r_gbr_b_grb("interp_r_grb_b_grb");
		interp_r_gbr_b_grb(x, y) = compute_clamp( 4*input(x-1, y) + ((input(x,y-2) + input(x,y+2))>>1) + 5*input(x,y) + 4*input(x+1,y), input(x-2,y) + input(x-1,y-1) + input(x-1,y+1) + input(x+1,y-1) + input(x+1,y+1) + input(x+2,y));
		// interpolate red pixels at green pixels, blue row, red colun
		Func interp_rgbr("interp_rgbr");
		interp_rgbr(x, y, c) = Halide::mux( ( (x+1)%2==0 && y%2==0 && c==0 ), {cast<u16>(0), interp_r_gbr_b_grb(x,y)} );
		// interpolate blue pixels at green pixels, red row, blue column
		Func interp_b_grb("interp_b_grb");
		interp_b_grb(x, y, c) = Halide::mux( ( x%2==0 && (y+1)%2==0 && c==2 ), {cast<u16>(0), interp_r_gbr_b_grb(x,y)} );

		Func interp_r_bbb_b_rrr("interp_r_bbb_b_rrr");
		interp_r_bbb_b_rrr(x, y) = compute_clamp_fractional_neg( 2*input(x-1,y-1) + 2*input(x-1,y+1) + 6*input(x,y) + 2*input(x+1,y-1) + 2*input(x+1,y+1), 3*input(x-2,y) + 3*input(x,y-2) + 3*input(x,y+2) + 3*input(x+2,y) );
		// interpolate red pixels at blue pixels, blue row, blue column
		Func interp_r_bbb("interp_r_bbb");
		interp_r_bbb(x, y, c) = Halide::mux( ( (x+1)%2==0 && (y+1)%2==0 && c==0 ), {cast<u16>(0), interp_r_bbb_b_rrr(x, y)} );
		// interpolate blue pixels at red pixels, red row, red column
		Func interp_b_rrr("interp_b_rrr");
		interp_b_rrr(x, y, c) = Halide::mux( ( x%2==0 && y%2==0 && c==2 ), {cast<u16>(0), interp_r_bbb_b_rrr(x, y)} );

		// write output
		debayered(x, y, c) = copy_red(x, y, c) + copy_green_TR(x, y, c) + copy_green_BL(x, y, c) + copy_blue(x, y, c) + interp_gr(x, y, c) + interp_gb(x, y, c) + interp_rg(x, y, c) + interp_bg(x, y, c) + interp_rgbr(x, y, c) + interp_b_grb(x, y, c) + interp_r_bbb(x, y, c) + interp_b_rrr(x, y, c);
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(PERFECTDebayer, Debayer)
