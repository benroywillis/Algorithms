#include "Halide.h"

namespace {

const int filter[9][9] = {
			 {1,   3,   4,   5,   6,   5,  4,    3,  1},
			 {3,   9,  12,  15,  18,  15,  12,   9,  3},
			 {4,  12,  16,  20,  24,  20,  16,  12,  4},
			 {5,  15,  20,  25,  30,  25,  20,  15,  5},
			 {6,  18,  24,  30,  36,  30,  24,  18,  6},
			 {5,  15,  20,  25,  30,  25,  20,  15,  5},
			 {4,  12,  16,  20,  24,  20,  16,  12,  4},
			 {3,   9,  12,  15,  18,  15,  12,   9,  3},
			 {1,   3,   4,   5,   6,   5,   4,   3,  1}
        };
halide_dimension_t f_dim[] = { {0, 9, 1}, {0, 9, 1} };


class TwoDConvolution : public Halide::Generator<TwoDConvolution> {
private:
	Buffer<int> fil{(int*)&filter, 2, f_dim};
public:
	// input image is grayscale 640x480
    Input<Buffer<int>> in{"in", 2};
    Output<Buffer<int>> TwoDConv{"TwoDConv", 2};

    void generate() {
		// induction over the input image
		Var x("x"), y("y");
		// to clamp the induction to the boundaries of the input image
		Func input("input");
		input(x, y) = in( clamp(x, 0, in.width()-1), clamp(y, 0, in.height()-1) );
		// Reduction over the convolution window
		RDom r(fil);
		// convolution over the fil
		Func conv("conv");
		conv(x, y) += fil(r.x, r.y) * input(x + r.x - 1, y + r.y - 1);		
		// output
		TwoDConv(x, y) = conv(x, y);
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(TwoDConvolution, TwoDConv)
