#include "Halide.h"
#include "halide_trace_config.h"
#include "halide_image_io.h"
#include <math.h>
#include "GEMM_config.h"

namespace {

class GeneralizedMatrixMultiply : public Halide::Generator<GeneralizedMatrixMultiply> {
public:
    Input<Buffer<PRECISION>> input0{"input0", 2};
    Input<Buffer<PRECISION>> input1{"input1", 2};
    Output<Buffer<PRECISION>> GEMM{"GEMM", 2};

    void generate() {
    	Var i("i"), j("j");
		RDom rv(0, input0.height());
		GEMM(j, i) = Halide::cast<PRECISION>(0);
		//GEMM(j, i) += input0(rv, i)*input1(j, rv);
		GEMM(j, i) += input0(rv, i)*input1(j, rv);
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(GeneralizedMatrixMultiply, GEMM)
