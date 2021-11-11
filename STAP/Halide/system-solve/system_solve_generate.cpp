#include "Halide.h"
#include <math.h>
#include <iostream>
#include "stap_utils.h"

#define M_PIf	(float)M_PI

namespace {

class SYSTEMSOLVE : public Halide::Generator<SYSTEMSOLVE> {
public:
	// d, b, ch_tdof0, ch_tdof1, c
	Input<Buffer<float>> covariances{"covariances", 5};
	// d, b, ch_tdof0, ch_tdof1, c
	Input<Buffer<float>> cholesky_factors{"cholesky_factors", 5};
	// st, ch_tdof0 (or 1), c
	Input<Buffer<float>> steering_vectors{"steering_vectors", 3};
	// d, b, st, ch_tdof0 (or 1), c
	Output<Buffer<float>> adaptive_weights{"adaptive_weights", 5};
	void generate() {
		// TDOF*N_CHAN, doppler bin, training blocks, steering vectors
		Var ch_tdof0("ch_tdof0"), ch_tdof1("ch_tdof1"), d("d"), b("b"), st("st"), c("c");
		// passthrough for debugging
		adaptive_weights(d, b, st, ch_tdof0, c) = 0.0f;
	}
};

}  // namespace

HALIDE_REGISTER_GENERATOR(SYSTEMSOLVE, system_solve)
