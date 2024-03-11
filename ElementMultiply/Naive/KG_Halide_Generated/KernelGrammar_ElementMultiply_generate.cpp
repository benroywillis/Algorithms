#include <Halide.h>

using Halide::Generator;

// 2024-03-08 things manipulated by the user
// 1. Got rid of two generator param things that didn't belong
// 2. Inserted output param both at the start of the generator and on the last stage of the pipeline
// 3. Fixed "fx" to "*" bc the operator print was not built for Halide
// 4. Parameterized estimates to the SIZE parameter

#ifndef SIZE
#define SIZE 	512
#endif

class KernelGrammar_ElementMultiply : public Generator<KernelGrammar_ElementMultiply> {
public:
	//GeneratorParam<> collection26{ "+collection26", 0};
	//GeneratorParam<> collection27{ "+collection27", 0};
	Input<Buffer<float>> collection26{"collection26", 2};
	Input<Buffer<float>> collection27{"collection27", 2};
	Output<Buffer<float>> out{"out", 2};
	void generate() {
		Var var15("var15");
		Var var16("var16");

		Func collection30("collection30");
		collection30 = Halide::BoundaryConditions::repeat_edge(collection26);
		Func collection31("collection31");
		collection31 = Halide::BoundaryConditions::repeat_edge(collection27);

		Func expr29("expr29");
		expr29(var16, var15) =  collection30(var16, var15) * collection31(var16, var15);

		Func output("output");
		output(var16, var15) = expr29(var16, var15);
		out = output;

		collection26.set_estimates({ { 0, SIZE }, { 0, SIZE } });
		collection27.set_estimates({ { 0, SIZE }, { 0, SIZE } });
		out.set_estimates({ { 0, SIZE }, { 0, SIZE } });
	}
};
HALIDE_REGISTER_GENERATOR(KernelGrammar_ElementMultiply, KernelGrammar_ElementMultiply)
