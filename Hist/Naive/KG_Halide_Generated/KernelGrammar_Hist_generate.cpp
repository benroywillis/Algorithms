#include <Halide.h>

using Halide::Generator;

// things that had to be helped by the user
// 1. the input and output were the same base pointer... this has to be helped in halide because you can't have the same things be the input and output
//    - so what we did was name the input "gry_img" and made the output "hist"
// 2. the generator says hist should have two dimensions... but it doesn't. so that had to be changed
// 3. implement the bp8 "pure function" - which instantiates the histogram to 0
//    - this is a Halide specific thing that needs to be explicitly stated for the front-end to know what we're talking about
// 4. replaced all the static parameters with SIZE or TYPE

#ifndef SIZE
#define SIZE 512
#endif

#ifndef PRECISION
#define TYPE uint32_t
#define TYPE_MAX 2147483647
#elif PRECISION == 0
#define TYPE uint32_t
#define TYPE_MAX 2147483647
#elif PRECISION == 1
#define TYPE uint16_t
#define TYPE_MAX 32767
#elif PRECISION == 2
#define TYPE uint8_t
#define TYPE_MAX 255
#endif

class KernelGrammar_Hist : public Generator<KernelGrammar_Hist> {
public:
	Input<Buffer<TYPE>> gry_img{"gry_img", 2};
	// this should only have one dimension... but the halide generator is confused right now
	//Output<Buffer<TYPE>> bp8{"bp8", 2};
	Output<Buffer<TYPE>> hist{"hist", 1};
	void generate() {
		Var var6("var6");
		Var var7("var7");

		Func bp14("bp14");
		bp14 = Halide::BoundaryConditions::repeat_edge(gry_img);

		Func expr13("expr13");
		expr13(var6) = Halide::cast<TYPE>(0);
		// below was the original histogram statement
		//expr13(var6) =  bp14(var6) + Halide::Expr(1);
		// this is changed to be halide-like
		// - RDOM is necessary because Halide needs a solid iterator domain to understand the histogram
		// - Halide::clamp is necessary because it will create an unbounded FGT task graph otherwise
		RDom r(0, SIZE, 0, SIZE);
		expr13(Halide::cast<int>( Halide::clamp(bp14(r.y, r.x), 0, TYPE_MAX) )) += Halide::cast<TYPE>(Halide::Expr(1));

		Func output("output");
		output(var6) = expr13(var6);
		hist = output;
		// below is the original estimate for the input size, we had to change it to the new input name
		//hist.set_estimates({ { 0, SIZE } });
		gry_img.set_estimates({ { 0, SIZE }, { 0, SIZE} });
		hist.set_estimates({ { 0, TYPE_MAX } });

		// manual schedule
		if( !using_autoscheduler() ) {
			// 2024-06-02 it has been found that the adams2019 schedulerr is not able to parallelize this task on its own
			// thus, we are manually scheduling the pipeline with reference to the hist app in the Halide repo:
			// https://github.com/halide/Halide/blob/7ca95d8658db5383325b7ca51cef21aaeaab89ca/apps/hist/hist_generator.cpp#L174C1-L208C44
			const int vecSize = natural_vector_size<TYPE>();
			expr13.update(0).unscheduled(); // unschedule the pure function
			expr13.in() // create a "wrapper" (special instance) of expr13
				  .compute_root() // and schedule this special instance to complete itself
				  .vectorize(var6, vecSize) // and vectorize its execution
				  .parallel(var6, 8);
			//expr13.compute_at(expr13.in(), r) // now schedule the histogram instances at the 
		}
	}
};
HALIDE_REGISTER_GENERATOR(KernelGrammar_Hist, KernelGrammar_Hist)
