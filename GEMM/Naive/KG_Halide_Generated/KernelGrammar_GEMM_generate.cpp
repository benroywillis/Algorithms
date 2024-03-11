#include <Halide.h>

// user modifications
// 1. Inject pipeline output code (includes the estimates)
// 2. added the size parameter for tuning purposes (and replaced all references to it - the input app had size at 64 so all those were replaced)

#ifndef SIZE
#define SIZE 64
#endif

using Halide::Generator;

class KernelGrammar_GEMM : public Generator<KernelGrammar_GEMM> {
public:
	Input<Buffer<float>> collection33{"collection33", 2};
	Input<Buffer<float>> collection34{"collection34", 2};
	Output<Buffer<float>> out{"out", 2};
	void generate() {
		Var var21("var21");
		Var var20("var20");
		Var var22("var22");

		//Func collection36("collection36"); 
		Func collection36 = Halide::BoundaryConditions::repeat_edge(collection33);
		///Func collection37("collection37");
		Func collection37 = Halide::BoundaryConditions::repeat_edge(collection34);

		RDom rv23(0, SIZE);
		Func call35("call35");
		call35(var22, var21) += collection36(var22, rv23) * collection37(rv23, var21);

		Func output("output");
		output = call35;

		out = output;

		collection33.set_estimates({ { 0, SIZE }, { 0, SIZE } });
		collection34.set_estimates({ { 0, SIZE }, { 0, SIZE } });
		out.set_estimates({ { 0, SIZE }, { 0, SIZE } });
	}
};
HALIDE_REGISTER_GENERATOR(KernelGrammar_GEMM, KernelGrammar_GEMM)
