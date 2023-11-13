#include <Halide.h>
#include "Halide/include/complex.h"

// 0 for float, 1 for double
#ifndef PRECISION
#define PRECISION 0
#endif

#if PRECISION == 0
#define TYPE float
#else
#define TYPE double
#endif


#ifndef SIZE
#define SIZE 512
#endif

using Halide::Generator;

// complex-to-complex matrix multiply
class ComplexMatrixMultiply : public Generator<ComplexMatrixMultiply> {
public:
	GeneratorParam<int> N{"N", SIZE};
	GeneratorParam<int> M{"M", SIZE};
	Input<Buffer<TYPE>> A{"A", 3};
	Input<Buffer<TYPE>> B{"B", 3};
	Output<Buffer<TYPE>> out{"out", 3};
	void generate() {
		Var x("x"), xi("xi"), xo("xo"), y("y"), yo("yo"), yi("yi"), yii("yii"), xii("xii"), c("c"), z("z");
		RDom k(0, M);

		ComplexFunc in0("in0");
		in0(x, y) = ComplexExpr( A(x, y, 0), A(x, y, 1) );
		ComplexFunc in1("in1");
		in1(x, y) = ComplexExpr( B(x, y, 0), B(x, y, 1) );

		ComplexFunc complex_row_col("complex_row_col");
		complex_row_col(x, y, z) = in0(z, y) * in1(x, z);
		ComplexFunc matrix_mul("matrix_mul");
		//matrix_mul(x, y) += in0(k, y)*in1(x, k);
		//matrix_mul(x, y) = ComplexExpr(0, 0);
		matrix_mul(x, y) = sum(complex_row_col(x, y, k));
		complex_row_col.compute_at(matrix_mul, y);

		ComplexFunc complexOutput("complexOutput");
		complexOutput(x, y) = matrix_mul(x, y);
		complexOutput.compute_root();

		complexOutput.bound(x, 0, N);
		complexOutput.bound(y, 0, M);
		out(x, y, c) = mux( c , { re(complexOutput(x, y)), im(complexOutput(x, y)) } );

		Var xy;
		complexOutput.tile(x, y, xi, yi, 32, 32).fuse(x, y, xy).parallel(xy).split(yi, yi, yii, 4).vectorize(xi, 8).unroll(xi).unroll(yii);

		matrix_mul.compute_at(complexOutput, yi).vectorize(x, 8).unroll(x).unroll(y);
	}
};

HALIDE_REGISTER_GENERATOR(ComplexMatrixMultiply, ComplexMatrixMultiply)
