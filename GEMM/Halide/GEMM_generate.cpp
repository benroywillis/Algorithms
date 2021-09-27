#include "Halide.h"
#include "halide_trace_config.h"
#include "halide_image_io.h"
#include <math.h>

namespace {

class GeneralizedMatrixMultiply : public Halide::Generator<GeneralizedMatrixMultiply> {
public:
    Input<Buffer<double>> input0{"input0", 2};
    Input<Buffer<double>> input1{"input1", 2};
    Output<Buffer<double>> GEMM{"GEMM", 2};

    void generate() {
    	Var i("i"), j("j"), k("k");
		Func inner_loop("inner_loop");
		inner_loop(k, i, j) = input0(i, k)*input1(k, j);

		RDom rv(0, input0.height());
		Func output("output");
		output(i, j) += inner_loop(rv, i, j);
		GEMM = output;
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(GeneralizedMatrixMultiply, GEMM)
