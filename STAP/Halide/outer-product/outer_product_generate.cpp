#include "Halide.h"
#include <math.h>
#include <iostream>
#include "stap_utils.h"

#define M_PIf	(float)M_PI

namespace {

class OUTERPRODUCT : public Halide::Generator<OUTERPRODUCT> {
public:
	// Chan, DOP, range, complex
	Input<Buffer<double>> datacube{"datacube", 4};
	// DOP, blocks, Chan*TDOF, Chan*TDOF, complex
	Output<Buffer<double>> covariances{"covariances", 5};
	void generate() {
		// induction variables over channel, blocks, steering, dopplar, range and complex
		Var ch("ch"), ch2("ch2"), b("b"), st("st"), d("d"), r("r"), c("c");
		// passthrough for debugging
		covariances(d, b, ch, ch2, c) = Halide::cast<double>(0);
	}
};

}  // namespace

HALIDE_REGISTER_GENERATOR(OUTERPRODUCT, outer_product)
