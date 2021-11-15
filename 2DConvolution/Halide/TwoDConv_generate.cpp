#include "Halide.h"

namespace {

const float filter[9][9] = {
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
//halide_dimension_t f_dim[] = { {0, 9, 1}, {0, 9, 1} };

class TwoDConvolution : public Halide::Generator<TwoDConvolution> {
private:
	Buffer<float> fil{(float*)&filter, 9, 9};
public:
	// input image is grayscale 640x480 and multiple frames
    Input<Buffer<int>> in{"in", 3};
    Output<Buffer<int>> TwoDConv{"TwoDConv", 3};

    void generate() {
		// induction over the input image frames
		Var x("x"), y("y"), f("f"), k("k"), l("l");
		// when the array access vars go out of bounds, the nearest pixel is returned
		// there's also "mirror_image" which walks back into the image for each index you go out of the image
		Func clamped = Halide::BoundaryConditions::repeat_edge(in);
		Func input("input");
		input(f, x, y) = clamped( y, x, f );
		Func filt("filt");
		filt(k, l) = fil(l, k) / 1024.0f;
		// Reduction over the convolution window
		RDom r(fil);
		// convolution over the fil
		Func conv("conv");
		conv(f, x, y) += filt(r.x, r.y) * Halide::cast<float>(input(f, x + r.x - 4, y + r.y - 4));
		// output
		TwoDConv(y, x, f) = Halide::cast<int>(conv(f, x, y));
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(TwoDConvolution, TwoDConv)
