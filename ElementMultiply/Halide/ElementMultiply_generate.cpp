#include <Halide.h>

#ifndef SIZE
#define SIZE 512
#endif

using Halide::Generator;
using Halide::RVar;
using Halide::ConciseCasts::i32;
using Halide::ConciseCasts::u16;
using Halide::ConciseCasts::u32;
using Halide::ConciseCasts::u8_sat;

class ElementMultiply : public Generator<ElementMultiply> {
public:
	GeneratorParam<int> N{"N", SIZE};
	GeneratorParam<int> M{"M", SIZE};
	Input<Buffer<float>> A{"A", 2};
	Input<Buffer<float>> B{"B", 2};
	Output<Buffer<float>> out{"out", 2};
	void generate() {
		Var x("x"), xi("xi"), xo("xo"), y("y"), yo("yo"), yi("yi"), yii("yii"), xii("xii");

		Func elem_mul("elem_mul");
		elem_mul(y, x) = A(y, x) * B(y, x);

		Func output;
		output(y, x) = elem_mul(y, x);

		Var xy;
		output.tile( x, y, xi, yi, 32, 32).fuse(x, y, xy).parallel(xy).split(yi, yi, yii, 4).vectorize(xi, 8).unroll(xi).unroll(yii);

		elem_mul.compute_at(output, yi).vectorize(x, 8).unroll(y);
		//elem_mul.update(0).reorder(x, y, k).vectorize(x, 8).unroll(x).unroll(y).unroll(k, 2);

		output.bound(x, 0, N);
		output.bound(y, 0, M);
		out = output;
	}
};

HALIDE_REGISTER_GENERATOR(ElementMultiply, ElementMultiply)
