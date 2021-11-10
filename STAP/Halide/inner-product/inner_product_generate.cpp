#include "Halide.h"
#include <math.h>
#include <iostream>
#include "stap_utils.h"

#define M_PIf	(float)M_PI

namespace {

class INNERPRODUCT : public Halide::Generator<INNERPRODUCT> {
public:
	// Chan, DOP, range, complex
	Input<Buffer<double>> datacube{"datacube", 4};
	// DOP, Blocks, steering, Chan*TDOF, complex
	Input<Buffer<double>> adaptive_weights{"adaptive_weights", 5};
	// steering, Chan*TDOF, copmlex
	Input<Buffer<double>> steering_vectors{"steering_vectors", 3};
	// steering, DOP, range, complex
	Output<Buffer<double>> output{"output", 4};
	void generate() {
		// induction variables over channel, blocks, steering, dopplar, range and complex
		Var ch("ch"), b("b"), st("st"), d("d"), r("r"), c("c");
		// passthrough for debugging
		output = datacube;
	}
};

}  // namespace

HALIDE_REGISTER_GENERATOR(INNERPRODUCT, inner_product)
