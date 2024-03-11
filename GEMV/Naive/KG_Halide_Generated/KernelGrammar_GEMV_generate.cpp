#include <Halide.h>

using Halide::Generator;

// 2024-03-08 User annotations to the code 
// 1. implemented the size parameter for speed testing
// 2. changed the BoundaryConditions:: statements to be compliant with Halide syntax
// 3. changed the GEMV reduction statement to use the RV correctly

#ifndef SIZE
#define SIZE 512
#endif

class KernelGrammar_GEMV : public Generator<KernelGrammar_GEMV> {
public:
	Input<Buffer<float>> collection26{"collection26", 2};
	Input<Buffer<float>> collection27{"collection27", 1};
	Output<Buffer<float>> collection25{"collection25", 1};
	void generate() {
		Var var14("var14");
		Var var13("var13");

		Func collection29("collection29");
		collection29 = Halide::BoundaryConditions::repeat_edge(collection26);
		Func collection30("collection30");
		collection30 = Halide::BoundaryConditions::repeat_edge(collection27);

		RDom rv15(0, SIZE);
		Func call28("call28");
//		call28(var14) += collection29(var14phi0, rv15phi0) * collection30(rv15phi0);
		call28(var14) += collection29(var14, rv15) * collection30(rv15);

		Func output("output");
		output(var14) = call28(var14);
		collection25 = output;
		collection26.set_estimates({ { 0, SIZE }, { 0, SIZE } });
		collection27.set_estimates({ { 0, SIZE } });
		collection25.set_estimates({ { 0, SIZE } });
	}
};
HALIDE_REGISTER_GENERATOR(KernelGrammar_GEMV, KernelGrammar_GEMV)
