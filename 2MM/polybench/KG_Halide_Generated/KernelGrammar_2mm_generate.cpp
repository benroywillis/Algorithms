#include <Halide.h>

#ifndef SIZE
#define SIZE 1024
#endif

using Halide::Generator;

class KernelGrammar_2mm : public Generator<KernelGrammar_2mm> {
public:
	Input<Buffer<double>> collection83{"collection83", 2};
	Input<Buffer<double>> collection84{"collection84", 2};
	Input<Buffer<double>> collection98{"collection98", 2};
	Input<Buffer<double>> collection100{"collection100", 2};
	// had to be changed by the user from "collection98" to "out"
	Output<Buffer<double>> out{"out", 2};
	void generate() {
		Var var73("var73");
		Var var89("var89");
		Var var74("var74");
		Var var72("var72");
		Var var88("var88");
		Var var90("var90");

		Func collection106("collection106");
		collection106 = Halide::BoundaryConditions::repeat_edge(collection83);
		Func collection107("collection107");
		collection107 = Halide::BoundaryConditions::repeat_edge(collection84);
		Func collection108("collection108");
		collection108 = Halide::BoundaryConditions::repeat_edge(collection98);
		Func collection109("collection109");
		collection109 = Halide::BoundaryConditions::repeat_edge(collection100);

		RDom rv75(0, SIZE);
		Func call87("call87");
		call87(var72, var74) +=  collection106(var72, rv75) * Halide::Expr(32412.000000) * collection107(rv75, var74);

		RDom rv91(0, SIZE);
		Func expr105("expr105");
		// had to change references from var88 and var90 to var72 and var74 respectively
		expr105(var72, var74) +=  call87(var72, rv91) * collection109(rv91, var74) +  collection108(var72, var74) * Halide::Expr(2123.000000);

		Func output("output");
		output(var72, var74) = expr105(var72, var74);
		out = output;
		collection83.set_estimates({ { 0, SIZE }, { 0, SIZE } });
		collection84.set_estimates({ { 0, SIZE }, { 0, SIZE } });
		collection98.set_estimates({ { 0, SIZE }, { 0, SIZE } });
		collection100.set_estimates({ { 0, SIZE }, { 0, SIZE } });
		out.set_estimates({ { 0, SIZE }, { 0, SIZE } });
	}
};
HALIDE_REGISTER_GENERATOR(KernelGrammar_2mm, KernelGrammar_2mm)
