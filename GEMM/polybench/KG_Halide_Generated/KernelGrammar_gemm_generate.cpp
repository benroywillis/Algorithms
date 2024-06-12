#include <Halide.h>

#ifndef SIZE
#define SIZE 512
#endif

using Halide::Generator;

class KernelGrammar_gemm : public Generator<KernelGrammar_gemm> {
public:
	Input<Buffer<double>> collection62{"collection62", 2};
	Input<Buffer<double>> collection63{"collection63", 2};
	Input<Buffer<double>> collection64{"collection64", 2};
	Output<Buffer<double>> collection61{"collection61", 2};
	void generate() {
		Var var52("var52");
		Var var53("var53");
		Var var51("var51");

		Func collection74("collection74");
		collection74 = Halide::BoundaryConditions::repeat_edge(collection62);
		Func collection75("collection75");
		collection75 = Halide::BoundaryConditions::repeat_edge(collection63);
		Func collection76("collection76");
		collection76 = Halide::BoundaryConditions::repeat_edge(collection64);

		RDom rv54(0, SIZE);
		Func expr72("expr72");
		expr72(var53, var52) += collection74(var53, rv54) * Halide::Expr(1.5) * collection75(rv54, var52) + Halide::Expr(1.2)*collection76(var53, var52);

		Func output("output");
		output(var53, var52) = expr72(var53, var52);
		collection61 = output;
		collection61.set_estimates({ { 0, SIZE }, { 0, SIZE } });
		collection62.set_estimates({ { 0, SIZE }, { 0, SIZE } });
		collection63.set_estimates({ { 0, SIZE }, { 0, SIZE } });
		collection64.set_estimates({ { 0, SIZE }, { 0, SIZE } });
	}
};
HALIDE_REGISTER_GENERATOR(KernelGrammar_gemm, KernelGrammar_gemm)
