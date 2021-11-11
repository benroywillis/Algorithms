#include "Halide.h"
#include <math.h>
#include <iostream>
#include "stap_utils.h"

#define M_PIf	(float)M_PI

namespace {

class OUTERPRODUCT : public Halide::Generator<OUTERPRODUCT> {
public:
	// Chan, DOP, range, complex
	Input<Buffer<float>> datacube{"datacube", 4};
	// DOP, blocks, Chan*TDOF, Chan*TDOF, complex
	Output<Buffer<float>> covariances{"covariances", 5};
	void generate() {
		// induction variables over channel, channel*(time-degree-of-freedom)0, channel*(time-degree-of-freedom)1, time degree of freedom, doppler+(tdof), training blocks, doppler bins, range bins, and complex
		//Var ch("ch"), ch_dof0("ch_dof0"), ch_dof1("ch_dof1"), tdof("tdof"), d_tdof("d_tdof"), b("b"), d("d"), r("r"), c("c");
		Var ch0("ch0"), ch1("ch1"), tdof0("tdof0"), tdof1("tdof1"), d("d"), b("b"), c("c"); 
		Var ch_dof0("ch_dof0"), ch_dof1("ch_dof1"), d_tdof("d_tdof");
		Func dop_index("dop_index");
		dop_index(d_tdof) = Halide::mux( (d_tdof >= 0 ) && (d_tdof < N_DOP), { 0, d_tdof } );

        Func accum_outer_product_lower("accum_outer_product_lower");
        accum_outer_product_lower(ch0, ch1, tdof0, tdof1, d, b, c) = 0.0f;
		RDom cell(0, TRAINING_BLOCK_SIZE);
        // cmult = cconj(adaptive_weight, steering_vector) = aw_re*sv_re - (-aw_im*sv_im) + j(aw_re*sv_im + -aw_im*sv_re)
        // = aw_re*sv_re + aw_im*sv_im + j(aw_re*sv_im - aw_im*sv_re)
        accum_outer_product_lower(ch0, ch1, tdof0, tdof1, d, b, c) += Halide::mux( c,
            { datacube(ch0, dop_index(d+tdof0-((TDOF-1)/2)), b*TRAINING_BLOCK_SIZE+cell, 0)*
			  datacube(ch1, dop_index(d+tdof1-((TDOF-1)/2)), b*TRAINING_BLOCK_SIZE+cell, 0)+
              datacube(ch0, dop_index(d+tdof0-((TDOF-1)/2)), b*TRAINING_BLOCK_SIZE+cell, 1)*
			  datacube(ch1, dop_index(d+tdof1-((TDOF-1)/2)), b*TRAINING_BLOCK_SIZE+cell, 1),
              datacube(ch0, dop_index(d+tdof0-((TDOF-1)/2)), b*TRAINING_BLOCK_SIZE+cell, 0)*
			  datacube(ch1, dop_index(d+tdof1-((TDOF-1)/2)), b*TRAINING_BLOCK_SIZE+cell, 1)-
              datacube(ch0, dop_index(d+tdof0-((TDOF-1)/2)), b*TRAINING_BLOCK_SIZE+cell, 1)*
			  datacube(ch1, dop_index(d+tdof1-((TDOF-1)/2)), b*TRAINING_BLOCK_SIZE+cell, 0) } );
		// accumulate outer products into covariances
		RDom i(0, N_CHAN, 0, TDOF, 0, N_CHAN, 0, TDOF);
		Func accum_over_cells("accum_over_cells");
		accum_over_cells(d, b, ch_dof0, ch_dof1, c) = 0.0f;
		accum_over_cells(d, b, i.x*N_CHAN+i.y, i.z*N_CHAN+i.w, c) = Halide::mux( ((i.z*N_CHAN+i.w) <= (i.x*N_CHAN+i.y)), { 0.0f, accum_outer_product_lower(i.x, i.z, i.y, i.w, d, b, c) } );
		// put the complex conjugate of the bottom-left covariances in the top right (conjugate symmetry)
		RDom j(0, N_CHAN, 0, TDOF, 0, N_CHAN, 0, TDOF);
		accum_over_cells(d, b, j.x*N_CHAN+j.y, j.z*N_CHAN+j.w, c) = Halide::mux( c, { accum_over_cells(d, b, j.x*N_CHAN+j.y, j.z*N_CHAN+j.w, c), -1.0f*accum_over_cells(d, b, j.x*N_CHAN+j.y, j.z*N_CHAN+j.w, c) } );
		// normalize all covariances
		RDom k(0, N_CHAN*TDOF, 0, N_CHAN*TDOF);
		accum_over_cells(d, b, k.x, k.y, c) /= Halide::cast<float>(TRAINING_BLOCK_SIZE);
		// write output
		covariances = accum_over_cells;
	}
};

}  // namespace

HALIDE_REGISTER_GENERATOR(OUTERPRODUCT, outer_product)

/*
input: 
input_doppler
input_range
for ch = 0; ch < N_CHAN; i++ 
  for tdof = input_dopper; tdof < TDOF+input_doppler; j++
    for k = 0; k <=ch*N_CHAN+tdof; k++
      complex x = datacube[ch][tdof+k][input_range]
*/
