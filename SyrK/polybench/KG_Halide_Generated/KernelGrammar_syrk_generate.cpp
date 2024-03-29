#include <Halide.h>

#ifndef SIZE
#define SIZE 1024
#endif

using Halide::Generator;

class KernelGrammar_syrk : public Generator<KernelGrammar_syrk> {
public:
	Input<Buffer<double>> bp2{"bp2", 2};
	Input<Buffer<double>> bp47{"bp47", 2};
	Output<Buffer<double>> bp46{"bp46", 2};
	void generate() {
		Var var0("var0");
		Var var1("var1");
		Var var42("var42");
		Var var43("var43");
		Var var44("var44");

		Func bp57("bp57");
		bp57 = Halide::BoundaryConditions::repeat_edge(bp2);
		Func bp59("bp59");
		bp59 = Halide::BoundaryConditions::repeat_edge(bp47);

		Func expr7("expr7");
		expr7(var1, var0) =  bp57(var1, var0) * Halide::Expr(2123.000000);

		RDom rv45(0, SIZE);
		//Func call56("call56");
		//call56(var44, var43) +=  bp59(var44, rv45) f* Halide::Expr(32412.000000) * bp59(var43, rv45);
		//expr7(var44, var43) = expr7(var44, var43) + bp59(var44, rv45) * Halide::Expr(32412.000000) * bp59(var43, rv45);
		//expr7(var44, var43) += bp59(var44, rv45) * Halide::Expr(32412.000000) * bp59(var43, rv45);
		expr7(var1, var0) += bp59(var1, rv45) * Halide::Expr(32412.000000) * bp59(var0, rv45);

		Func output("output");
		//output(var44, var43) = call56(var44, var43);
		//output(var44, var43) = expr7(var44, var43);
		output(var1, var0) = expr7(var1, var0);
		bp46 = output;
		bp2.set_estimates({ { 0, SIZE }, { 0, SIZE } });
		bp47.set_estimates({ { 0, SIZE }, { 0, SIZE } });
		bp46.set_estimates({ { 0, SIZE }, { 0, SIZE } });
	}
};
HALIDE_REGISTER_GENERATOR(KernelGrammar_syrk, KernelGrammar_syrk)
