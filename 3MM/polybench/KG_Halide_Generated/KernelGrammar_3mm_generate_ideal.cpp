#include <Halide.h>

#ifndef SIZE
#define SIZE 1024
#endif

using Halide::Generator;

class KernelGrammar_3mm : public Generator<KernelGrammar_3mm> {
public:

	Input<Buffer<double>> collection83{"collection83", 2};
	Input<Buffer<double>> collection84{"collection84", 2};
	Input<Buffer<double>> collection97{"collection97", 2};
	Input<Buffer<double>> collection98{"collection98", 2};
	Output<Buffer<double>> collection110{"collection110", 2};
	void generate() {
		Var var72("var72");
		Var var86("var86");
		Var var87("var87");
		Var var74("var74");
		Var var73("var73");
		Var var88("var88");
		Var var100("var100");
		Var var101("var101");
		Var var102("var102");

		Func collection114("collection114");
		collection114 = Halide::BoundaryConditions::repeat_edge(collection83);
		Func collection115("collection115");
		collection115 = Halide::BoundaryConditions::repeat_edge(collection84);
		Func collection116("collection116");
		collection116 = Halide::BoundaryConditions::repeat_edge(collection97);
		Func collection117("collection117");
		collection117 = Halide::BoundaryConditions::repeat_edge(collection98);

		RDom rv75(0, SIZE);
		Func call85("call85");
		call85(var72, var74) += collection114(var72, rv75) * collection115(rv75, var74);

		RDom rv89(0, SIZE);
		Func call99("call99");
		// this should refer to the same polyhedral space as everyone else
		//call99(var86, var88) += collection116(var86, rv89) * collection117(rv89, var88);
		call99(var72, var74) += collection116(var72, rv89) * collection117(rv89, var74);

		RDom rv103(0, SIZE);
		Func call113("call113");
		// this contains missing vars (var72) and doesn't refer to the reduction (rv103)
		//call113(var86, var88) += call85(var72, var74) * call99(var86, var88);
		call113(var72, var74) += call85(var72, rv103) * call99(rv103, var74);

		Func output("output");
		// this should refer to the vars from above
		//output(var86, var88) = call113(var86, var88);
		output(var72, var74) = call113(var72, var74);
		collection110 = output;
		collection83.set_estimates({ { 0, SIZE }, { 0, SIZE } });
		collection84.set_estimates({ { 0, SIZE }, { 0, SIZE } });
		collection97.set_estimates({ { 0, SIZE }, { 0, SIZE } });
		collection98.set_estimates({ { 0, SIZE }, { 0, SIZE } });
		collection110.set_estimates({ { 0, SIZE }, { 0, SIZE } });
	}
};
HALIDE_REGISTER_GENERATOR(KernelGrammar_3mm, KernelGrammar_3mm)
