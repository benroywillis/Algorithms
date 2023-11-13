#include <Halide.h>

#ifndef SIZE
#define SIZE 512
#endif

using Halide::Generator;

class MatrixMultiply : public Generator<MatrixMultiply> {
public:
	GeneratorParam<int> N{"N", SIZE};
	GeneratorParam<int> M{"M", SIZE};
	Input<Buffer<float>> A{"A", 2};
	Input<Buffer<float>> B{"B", 2};
	Output<Buffer<float>> out{"out", 2};
	void generate() {
		Var x("x"), xi("xi"), xo("xo"), y("y"), yo("yo"), yi("yi"), yii("yii"), xii("xii");

		RDom k(0, M);

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
