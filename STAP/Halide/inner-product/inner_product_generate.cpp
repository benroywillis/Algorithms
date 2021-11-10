#include "Halide.h"
#include <math.h>
#include <iostream>
#include "stap_utils.h"

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
		// induction variables over antenna channel, temporal degree of freedom, training blocks, steering vector, doppler bin, range cell and complex
		Var ch("ch"), tdof("tdof"), b("b"), st("st"), d("d"), r("r"), c("c"), d_tdof("d_tdof");

		Func accum_gamma_weight("accum_gamma_weight");
		accum_gamma_weight(d, b, st, c) = 0.0f;
		RDom i(0, N_CHAN*TDOF);
		// cmult = cconj(adaptive_weight, steering_vector) = aw_re*sv_re - (-aw_im*sv_im) + j(aw_re*sv_im + -aw_im*sv_re)
		// = aw_re*sv_re + aw_im*sv_im + j(aw_re*sv_im - aw_im*sv_re)
		accum_gamma_weight(d, b, st, 0) += adaptive_weights(d, b, st, i, 0)*steering_vectors(st, i, 0)+
										   adaptive_weights(d, b, st, i, 1)*steering_vectors(st, i, 1);
		accum_gamma_weight(d, b, st, 1) += adaptive_weights(d, b, st, i, 0)*steering_vectors(st, i, 1)-
										   adaptive_weights(d, b, st, i, 1)*steering_vectors(st, i, 0);
		Func compute_gamma_weight("compute_gamma_weight");
		compute_gamma_weight(d, b, st) = Halide::sqrt(accum_gamma_weight(d, b, st, 0)*accum_gamma_weight(d, b, st, 0) + 
											          accum_gamma_weight(d, b, st, 1)*accum_gamma_weight(d, b, st, 1) );

		Func dop_index_offset("dop_index_offset");
		dop_index_offset(d_tdof) = Halide::mux( ((d_tdof - (TDOF-1)/2) < 0), { (d_tdof - (TDOF-1)/2) - N_DOP, (d_tdof - (TDOF-1)/2) + N_DOP } );
		Func dop_index("dop_index");
		dop_index(d_tdof) = Halide::mux( ((0 <= (d_tdof - (TDOF-1)/2)) && ((d_tdof - (TDOF-1)/2) < N_DOP)), { dop_index_offset(d_tdof), d_tdof-(TDOF-1)/2 } );
		Func accum_inner_product("accum_inner_product");
		accum_inner_product(ch, d, tdof, b, st, r, c) = 0.0f;
		RDom j(0, N_CHAN*TDOF);
		// cmult = cconj(adaptive_weight, steering_vector) = aw_re*sv_re - (-aw_im*sv_im) + j(aw_re*sv_im + -aw_im*sv_re)
		// = aw_re*sv_re + aw_im*sv_im + j(aw_re*sv_im - aw_im*sv_re)
		accum_inner_product(ch, d, tdof, b, st, r, c) += mux( c, 
			{ adaptive_weights(d, b, st, 0, 0)*datacube(ch, dop_index(d+tdof), j, 0)+
			  adaptive_weights(d, b, st, 0, 1)*datacube(ch, dop_index(d+tdof), j, 1),
			  adaptive_weights(d, b, st, 0, 0)*datacube(ch, dop_index(d+tdof), j, 1)-
			  adaptive_weights(d, b, st, 0, 1)*datacube(ch, dop_index(d+tdof), j, 0) } );

		Func compute_output("compute_output");
		compute_output(ch, d, tdof, b, st, r, c) = accum_inner_product(ch, d, tdof, b, st, r, c) * 1.0f / compute_gamma_weight(d, b, st);
		// reduction from the 7D input space to the 4D output space
		// 1st dim: channel space
		// 2nd dim: time degrees of freedom space
		// 3rd dim: training block space
		// 4th dim: first range cell to last range cell is TRAINING_BLOCK_SIZE in length, offset by block*TRAINING_BLOCK_SIZE
		RDom _red(0, N_CHAN, 0, TDOF, 0, N_BLOCKS, 0, TRAINING_BLOCK_SIZE);
		output(st, d, r, c) = 0.0f;
		output(st, d, _red.w, c) = compute_output(_red.x, d, _red.y, _red.z, st, _red.z*TRAINING_BLOCK_SIZE+_red.w, c);
 
	}
};

}  // namespace

HALIDE_REGISTER_GENERATOR(INNERPRODUCT, inner_product)
