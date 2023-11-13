#include "Halide.h"

namespace {

class INNERPRODUCT : public Halide::Generator<INNERPRODUCT> {
public:
	GeneratorParam<int> N{"N", 512};
	Input<Buffer<float>> in0{"in0", 1};
	Input<Buffer<float>> in1{"in1", 1};
	Output<float> output{"output"};
	void generate() {
		Var x;
		output(x) = 0.0f;
		RDom r(in0);
		output(x) += in0(r) * in1(r);
	}
};

}  // namespace

HALIDE_REGISTER_GENERATOR(INNERPRODUCT, inner_product)
