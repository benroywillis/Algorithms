#include <Halide.h>

using Halide::Generator;

class KernelGrammar_gemver : public Generator<KernelGrammar_gemver> {
public:
	Input<Buffer<double>> bp2{"bp2", 1};
	Input<Buffer<double>> bp12{"bp12", 1};
	Input<Buffer<double>> bp133{"bp133", 1};
	//Input<Buffer<double>> bp135{"bp135", 1};
	Input<Buffer<double>> bp134{"bp134", 1};
	//Input<Buffer<double>> bp136{"bp136", 1};
	Input<Buffer<double>> bp132{"bp132", 2};
	Output<Buffer<double>> bp25{"bp25", 1};
	void generate() {
		Var var22("var22");
		Var var21("var21");
		Var var7("var7");
		Var var8("var8");
		Var var0("var0");
		Var var130("var130");
		Var var131("var131");

		Func bp147("bp147");
		bp147 = Halide::BoundaryConditions::repeat_edge(bp2);
		Func bp149("bp149");
		bp149 = Halide::BoundaryConditions::repeat_edge(bp12);
		Func bp151("bp151");
		bp151 = Halide::BoundaryConditions::repeat_edge(bp133);
		//Func bp153("bp153");
		//bp153 = Halide::BoundaryConditions::repeat_edge(bp135);
		Func bp155("bp155");
		bp155 = Halide::BoundaryConditions::repeat_edge(bp134);
		//Func bp157("bp157");
		//bp157 = Halide::BoundaryConditions::repeat_edge(bp136);
		Func bp159("bp159");
		bp159 = Halide::BoundaryConditions::repeat_edge(bp132);

		Func expr146("expr146");
		expr146(var131, var130) =  bp159(var131, var130) + bp151(var131) * bp155(var130);

		RDom rv9(0, 500);
		Func call20("call20");
		call20(var8) +=  expr146(rv9, var8) * Halide::Expr(12313.000000) * bp149(rv9);

		Func expr6("expr6");
		expr6(var0) =  bp147(var0) + call20(var0);

		RDom rv23(0, 500);
		Func call34("call34");
		call34(var22) +=  expr146(var22, rv23) * Halide::Expr(43532.000000) * expr6(rv23);

		Func output("output");
		output(var22) = call34(var22);
		bp25 = output;
		bp2.set_estimates({ { 0, 500 } });
		bp12.set_estimates({ { 0, 500 } });
		bp133.set_estimates({ { 0, 500 } });
		//bp135.set_estimates({ { 0, 500 } });
		bp134.set_estimates({ { 0, 500 } });
		//bp136.set_estimates({ { 0, 500 } });
		bp132.set_estimates({ { 0, 500 }, { 0, 500 } });
		bp25.set_estimates({ { 0, 500 } });
	}
};
HALIDE_REGISTER_GENERATOR(KernelGrammar_gemver, KernelGrammar_gemver)
