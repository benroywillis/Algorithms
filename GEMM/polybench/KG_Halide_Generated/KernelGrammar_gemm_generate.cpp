#include <Halide.h>

using Halide::Generator;

class KernelGrammar_gemm : public Generator<KernelGrammar_gemm> {
public:
	Input<Buffer<double>> collection62{"collection62", 2};
	Input<Buffer<double>> collection63{"collection63", 2};
	Output<Buffer<double>> collection61{"collection61", 2};
	void generate() {
		Var var52("var52");
		Var var53("var53");
		Var var51("var51");

		Func collection74("collection74");
		collection74 = Halide::BoundaryConditions::repeat_edge(collection62);
		Func collection75("collection75");
		collection75 = Halide::BoundaryConditions::repeat_edge(collection63);

		RDom rv54(0, 1024);
		Func expr72("expr72");
		expr72(var53, var52) += collection74(var53, rv54) * Halide::Expr(32412.000000) * collection75(rv54, var52);

		Func output("output");
		output(var53, var52) = expr72(var53, var52);
		collection61 = output;
		collection61.set_estimates({ { 0, 1024 }, { 0, 1024 } });
		collection62.set_estimates({ { 0, 1024 }, { 0, 1024 } });
		collection63.set_estimates({ { 0, 1024 }, { 0, 1024 } });
		collection61.set_estimates({ { 0, 1024 }, { 0, 1024 } });
	}
};
HALIDE_REGISTER_GENERATOR(KernelGrammar_gemm, KernelGrammar_gemm)
