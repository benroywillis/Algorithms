#include "Halide.h"
#include <iostream>
#include <math.h>
#include "wami_debayer.h"

#define STDEV_THRESH 		2.5f
#define INIT_STDEV	 		80.0f
#define ALPHA 		 		0.01f
#define INIT_WEIGHT  		0.01f
#define BACKGROUND_THRESH   0.9f
#define ONE_OVER_SQRT_TWO_PI (1.0f / sqrt(2.0f * (float)M_PI))

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
		// clamps the iterators to the boundaries of the input arrays
		Func input("input");
		input(x, y) = bayer( clamp(x, 0, bayer.height()), clamp(y, 0, bayer.width()) ); 
		debayered(x, y, c) = input(x, y);
		
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(PERFECTDebayer, Debayer)
