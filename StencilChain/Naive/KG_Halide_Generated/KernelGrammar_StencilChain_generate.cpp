#include <Halide.h>

// 2024-03-08 User annotations
// 1. put input and output image params into generator beginning
// 2. put repeat_edge statements on the input
// 3. got rid of generatorparams at the start of the generator
// 4. Cyclebite-Template doesn't yet support uint8_t so we casted it to uint32_t
// 5. Each reference in the rgb2gray function (first one with the weights) needed to be changed to the input buffer
// 6. The 0th reference in the rgb2gray function (that gets the "r" pixel) needed to be added (it's simply missing from the generated code)
// 7. The input/output estimates needed to be added because the input and output buffers were missing
// 8. Input buffer bounding function had to be added because the input buffer was missing from the generated code

using Halide::Generator;

float const169[5][5] = { 
	{ 0.003663f, 0.014652f, 0.025641f, 0.014652f, 0.003663f },
	{ 0.014652f, 0.058608f, 0.095238f, 0.058608f, 0.014652f },
	{ 0.025641f, 0.095238f, 0.150183f, 0.095238f, 0.025641f },
	{ 0.014652f, 0.058608f, 0.095238f, 0.058608f, 0.014652f },
	{ 0.003663f, 0.014652f, 0.025641f, 0.014652f, 0.003663f } };
float const229[5][5] = { 
	{ 0.003663f, 0.014652f, 0.025641f, 0.014652f, 0.003663f },
	{ 0.014652f, 0.058608f, 0.095238f, 0.058608f, 0.014652f },
	{ 0.025641f, 0.095238f, 0.150183f, 0.095238f, 0.025641f },
	{ 0.014652f, 0.058608f, 0.095238f, 0.058608f, 0.014652f },
	{ 0.003663f, 0.014652f, 0.025641f, 0.014652f, 0.003663f } };
float const199[5][5] = { 
	{ 0.003663f, 0.014652f, 0.025641f, 0.014652f, 0.003663f },
	{ 0.014652f, 0.058608f, 0.095238f, 0.058608f, 0.014652f },
	{ 0.025641f, 0.095238f, 0.150183f, 0.095238f, 0.025641f },
	{ 0.014652f, 0.058608f, 0.095238f, 0.058608f, 0.014652f },
	{ 0.003663f, 0.014652f, 0.025641f, 0.014652f, 0.003663f } };
float const259[5][5] = { 
	{ 0.003663f, 0.014652f, 0.025641f, 0.014652f, 0.003663f },
	{ 0.014652f, 0.058608f, 0.095238f, 0.058608f, 0.014652f },
	{ 0.025641f, 0.095238f, 0.150183f, 0.095238f, 0.025641f },
	{ 0.014652f, 0.058608f, 0.095238f, 0.058608f, 0.014652f },
	{ 0.003663f, 0.014652f, 0.025641f, 0.014652f, 0.003663f } };
float const289[5][5] = { 
	{ 0.003663f, 0.014652f, 0.025641f, 0.014652f, 0.003663f },
	{ 0.014652f, 0.058608f, 0.095238f, 0.058608f, 0.014652f },
	{ 0.025641f, 0.095238f, 0.150183f, 0.095238f, 0.025641f },
	{ 0.014652f, 0.058608f, 0.095238f, 0.058608f, 0.014652f },
	{ 0.003663f, 0.014652f, 0.025641f, 0.014652f, 0.003663f } };

class KernelGrammar_StencilChain : public Generator<KernelGrammar_StencilChain> {
public:
	Input<Buffer<uint32_t>> in{"in", 3};
	Output<Buffer<float>> out{"out", 2};
	void generate() {
		Var var146("var146");
		Var var206("var206");
		Var var101("var101");
		Var var100("var100");
		Var var234("var234");
		Var var143("var143");
		Var var173("var173");
		Var var145("var145");
		Var var144("var144");
		Var var174("var174");
		Var var204("var204");
		Var var175("var175");
		Var var203("var203");
		Var var236("var236");
		Var var176("var176");
		Var var205("var205");
		Var var235("var235");
		Var var264("var264");
		Var var263("var263");
		Var var233("var233");
		Var var266("var266");
		Var var265("var265");

		Buffer<float,2> const169_buffer{ &const169[0][0],5,5 };
		Buffer<float,2> const229_buffer{ &const229[0][0],5,5 };
		Buffer<float,2> const199_buffer{ &const199[0][0],5,5 };
		Buffer<float,2> const259_buffer{ &const259[0][0],5,5 };
		Buffer<float,2> const289_buffer{ &const289[0][0],5,5 };

		Func bounded("bounded");
		bounded = Halide::BoundaryConditions::repeat_edge(in);

		Func expr142("expr142");
		expr142(var100, var101) =  Halide::cast<float>(bounded(var100, var101, 2)) * 0.114000f + Halide::cast<float>(bounded(var100, var101, 1)) * 0.587000f + Halide::cast<float>(bounded(var100, var101, 0)) * 0.299000f;

		RDom rv207(-2, 3, -2, 3);
		Func call232("call232");
		call232(var100, var101) +=  expr142(var100+rv207.x, var101+rv207.y) * const229_buffer(rv207.x+2, rv207.y+2);

		RDom rv147(-2, 3, -2, 3);
		Func call172("call172");
		call172(var100, var101) +=  call232(var100+rv147.x, var101+rv147.y) * const169_buffer(rv147.x+2, rv147.y+2);

		RDom rv267(-2, 3, -2, 3);
		Func call292("call292");
		call292(var100, var101) +=  call172(var100+rv267.x, var101+rv267.y) * const289_buffer(rv267.x+2, rv267.y+2);

		RDom rv237(-2, 3, -2, 3);
		Func call262("call262");
		call262(var100, var101) +=  call292(var100+rv237.x, var101+rv237.y) * const259_buffer(rv237.x+2, rv237.y+2);

		RDom rv177(-2, 3, -2, 3);
		Func call202("call202");
		call202(var100, var101) +=  call262(var100+rv177.x, var101+rv177.y) * const199_buffer(rv177.x+2, rv177.y+2);

		Func output("output");
		output(var100, var101) = call202(var100, var101);
		out = output;

		in.set_estimates({ {0, 1280}, {0, 1920}, {0, 2} });
		out.set_estimates({ {0, 1280}, {0, 1920} });
	}
};
HALIDE_REGISTER_GENERATOR(KernelGrammar_StencilChain, KernelGrammar_StencilChain)
