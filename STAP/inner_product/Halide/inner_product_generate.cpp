#include "Halide.h"
#include <math.h>
#include <iostream>
#include "stap_utils.h"
#include "include/complex.h"

#define M_PIf	(float)M_PI

namespace {

class INNERPRODUCT : public Halide::Generator<INNERPRODUCT> {
public:
	// antenna channel, doppler bin, range cell, complex
	Input<Buffer<float>> datacube{"datacube", 4};
	// doppler bin, training block, steering vector, (antenna channel*(temporal degree of freedom), complex
	Input<Buffer<float>> adaptive_weights{"adaptive_weights", 5};
	// steering vector, (antenna channel)*(temporal degree of freedom), complex
	Input<Buffer<float>> steering_vectors{"steering_vectors", 3};
	// steering vector, doppler bin, range bin, complex
	Output<Buffer<float>> output{"output", 4};
	void generate() {
		Func clamped = Halide::BoundaryConditions::mirror_image(datacube);
		// induction variables over antenna channel, temporal degree of freedom, training blocks, steering vector, doppler bin, range cell and complex
		Var ch("ch"), tdof("tdof"), b("b"), st("st"), d("d"), r("r"), c("c");

		ComplexFunc in_datacube("in_datacube");
		in_datacube(ch, d, r) = ComplexExpr( clamped( 0, r, d, ch ), clamped( 1, r, d, ch ) );
		ComplexFunc in_adaptive_weights("in_adaptive_weights");
		in_adaptive_weights(d, b, st, tdof) = ComplexExpr( adaptive_weights(0, tdof, st, b, d), adaptive_weights(1, tdof, st, b, d) );
		ComplexFunc in_steering_vectors("in_steering_vectors");
		in_steering_vectors(st, tdof) = ComplexExpr( steering_vectors(0, tdof, st), steering_vectors(1, tdof, st) );

		ComplexFunc accum_gamma_weight("accum_gamma_weight");
		accum_gamma_weight(d, b, st) = ComplexExpr();
		RDom i(0, N_CHAN*TDOF);
		accum_gamma_weight(d, b, st) += conj(in_adaptive_weights(d, b, st, i))*in_steering_vectors(st, i);
		accum_gamma_weight.compute_root();
		Func compute_gamma_weight("compute_gamma_weight");
		compute_gamma_weight(d, b, st) = Halide::sqrt(accum_gamma_weight(d, b, st).re()*accum_gamma_weight(d, b, st).re() + 
											          accum_gamma_weight(d, b, st).im()*accum_gamma_weight(d, b, st).im() );

		ComplexFunc accum_inner_product("accum_inner_product");
		accum_inner_product(d, b, st, r) = ComplexExpr();
		RDom j(0, N_CHAN, 0, TDOF);
		accum_inner_product(d, b, st, r) += conj(in_adaptive_weights(d, b, st, 0))*in_datacube(j.x, j.y+d-1, r);
		accum_inner_product.compute_root();

		ComplexFunc compute_output("compute_output");
		compute_output(d, b, st, r) = accum_inner_product(d, b, st, r) * 1.0f / compute_gamma_weight(d, b, st);
		// reduction from the 7D input space to the 4D output space
		// 1st dim: channel space
		// 2nd dim: time degrees of freedom space
		// 3rd dim: training block space
		// 4th dim: first range cell to last range cell is TRAINING_BLOCK_SIZE in length, offset by block*TRAINING_BLOCK_SIZE
		RDom _red(0, N_BLOCKS, 0, TRAINING_BLOCK_SIZE);
		output(c, r, d, st) = 0.0f;
		output(0, r, d, st) = compute_output(d, _red.x, st, _red.x*TRAINING_BLOCK_SIZE+_red.y).re(); 
		output(1, r, d, st) = compute_output(d, _red.x, st, _red.x*TRAINING_BLOCK_SIZE+_red.y).im(); 
	}
};

}  // namespace

HALIDE_REGISTER_GENERATOR(INNERPRODUCT, inner_product)
