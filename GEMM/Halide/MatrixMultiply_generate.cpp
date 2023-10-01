// This generator implements a quantized matrix multiplication and schedules for
// HVX and CPU. The generator schedule assumes certain size constraints on the
// two input matrices:
// * The width of the left-hand-side (mat_a_ below) must be divisible by 4.
// * The height of mat_a_ must be divisible by 4.
// * The width of the right-hand-side (mat_b_ below) must be divisible by
//   the natural vector size of the architecture you want to run this code on.
// Note that all these constraints are asserted at runtime, so running with
// illegal sizes with trigger those assertions. Correct input sizes can be
// achieved by padding mat_a_ with the value -mat_a_offset_ and mat_b_ with the
// value -mat_b_offset_.

#include <Halide.h>

using Halide::Generator;
using Halide::RVar;
using Halide::ConciseCasts::i32;
using Halide::ConciseCasts::u16;
using Halide::ConciseCasts::u32;
using Halide::ConciseCasts::u8_sat;

class MatrixMultiply : public Generator<MatrixMultiply> {
public:
	GeneratorParam<int> N{"N", 512};
	GeneratorParam<int> M{"M", 512};
	Input<Buffer<float>> A{"A", 2};
	Input<Buffer<float>> B{"B", 2};
	Output<Buffer<float>> out{"out", 2};
	void generate() {
		Var x("x"), xi("xi"), xo("xo"), y("y"), yo("yo"), yi("yi"), yii("yii"), xii("xii");

		RDom k(0, M);
		RVar ki;

		Func matrix_mul("matrix_mul");
		matrix_mul(x, y) += A(k, y) * B(x, k);

		Func output;
		output(x, y) = matrix_mul(x, y);

		Var xy;
		output.tile( x, y, xi, yi, 32, 32).fuse(x, y, xy).parallel(xy).split(yi, yi, yii, 4).vectorize(xi, 8).unroll(xi).unroll(yii);

		matrix_mul.compute_at(output, yi).vectorize(x, 8).unroll(y);
		matrix_mul.update(0).reorder(x, y, k).vectorize(x, 8).unroll(x).unroll(y).unroll(k, 2);

		output.bound(x, 0, N);
		output.bound(y, 0, M);
		out = output;
	}
};

HALIDE_REGISTER_GENERATOR(MatrixMultiply, MatrixMultiply)
