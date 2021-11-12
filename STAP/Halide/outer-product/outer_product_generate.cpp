#include "Halide.h"
#include <math.h>
#include <iostream>
#include "stap_utils.h"
#include "include/complex.h"

#define M_PIf	(float)M_PI

namespace {

class OUTERPRODUCT : public Halide::Generator<OUTERPRODUCT> {
public:
	// Chan, DOP, range, complex
	Input<Buffer<float>> datacube{"datacube", 4};
	// DOP, blocks, Chan*TDOF, Chan*TDOF, complex
	Output<Buffer<float>> covariances{"covariances", 5};
	void generate() {
		Var d("d"), b("b"), c_tdof0("c_tdof0"), c_tdof1("c_tdof1"), d_tdof("d_tdof"), ch("ch"), r("r"), c("c");
		Func clamped = Halide::BoundaryConditions::mirror_image(datacube);
		ComplexFunc in_datacube("in_datacube");
		in_datacube(ch, d, r) = ComplexExpr( clamped(ch, d, r, 0), clamped(ch, d, r, 1) );

		Func dop_index("dop_index");
		dop_index(d_tdof) = Halide::mux( (d_tdof >= 0 ) && (d_tdof < N_DOP), { 0, d_tdof } );

		// compute outer product between two points in the covariance matrix
		RDom cell(0, N_CHAN, 0, TDOF, 0, N_CHAN, 0, TDOF);
		cell.where( (cell.z*N_CHAN+cell.w) <= (cell.x*N_CHAN+cell.y) );
		ComplexFunc accum_outer_product_lower("accum_outer_product_lower");
		accum_outer_product_lower(d, b, c_tdof0, c_tdof1, r) = ComplexExpr();
        accum_outer_product_lower(d, b, cell.x*N_CHAN+cell.y, cell.z*N_CHAN+cell.w, r) += in_datacube(cell.x, dop_index(d - 1 + cell.y), r)*conj(in_datacube(cell.z, dop_index(d - 1 + cell.w), r));
		//accum_outer_product_lower.compute_root();

		// accumulate outer products into covariances
		RDom i(0, TRAINING_BLOCK_SIZE);
		ComplexFunc accum_over_cells("accum_over_cells");
		accum_over_cells(d, b, c_tdof0, c_tdof1) = ComplexExpr();
		accum_over_cells(d, b, c_tdof0, c_tdof1) += accum_outer_product_lower(d, b, c_tdof0, c_tdof1, b*TRAINING_BLOCK_SIZE+i);
		// put the complex conjugate of the bottom-left covariances in the top right (conjugate symmetry)
		RDom j(0, N_CHAN*TDOF, 0, N_CHAN*TDOF);
		j.where( j.y > j.x );
		accum_over_cells(d, b, j.y, j.x) = accum_over_cells(d, b, j.x, j.y);
		// normalize all covariances
		RDom k(0, N_CHAN*TDOF, 0, N_CHAN*TDOF);
		accum_over_cells(d, b, k.x, k.y) = accum_over_cells(d, b, k.x, k.y) / Halide::cast<float>(TRAINING_BLOCK_SIZE);
		accum_over_cells.compute_root();
		// write output
		covariances(d, b, c_tdof0, c_tdof1, c) = 0.0f;
		covariances(d, b, c_tdof0, c_tdof1, 0) = accum_over_cells(d, b, c_tdof0, c_tdof1).re();
		covariances(d, b, c_tdof0, c_tdof1, 1) = accum_over_cells(d, b, c_tdof0, c_tdof1).im();
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
