#include <Halide.h>

#ifndef SIZE
#define SIZE 512
#endif

using Halide::Generator;

class GEMV : public Generator<GEMV> {
public:
	GeneratorParam<int> N{"N", SIZE};
	GeneratorParam<int> M{"M", SIZE};
	Input<Buffer<float>> A{"A", 2};
	Input<Buffer<float>> B{"B", 1};
	Output<Buffer<float>> out{"out", 1};
	void generate() {
		Var y("y");
		RDom k(0, M);

		Func matrix_vector_mul("matrix_vector_mul");
		matrix_vector_mul(y) += A(y, k) * B(k);

		Func output;
		output(y) = matrix_vector_mul(y);

		Var y_inner, y_outer;
		output.split(y, y_outer, y_inner, SIZE/16).vectorize(y_inner, 8).parallel(y_outer);
		output.bound(y, 0, N);
		out = output;
	}
};

HALIDE_REGISTER_GENERATOR(GEMV, GEMV)
