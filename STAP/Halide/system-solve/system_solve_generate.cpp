#include "Halide.h"
#include <math.h>
#include <iostream>
#include "stap_utils.h"
#include "include/complex.h"

#define M_PIf	(float)M_PI

namespace {

class SYSTEMSOLVE : public Halide::Generator<SYSTEMSOLVE> {
public:
	// d, b, ch_tdof0, ch_tdof1, c
	Input<Buffer<float>> covariances{"covariances", 5};
	// d, b, ch_tdof0, ch_tdof1, c
	Input<Buffer<float>> cholesky_factors{"cholesky_factors", 5};
	// st, ch_tdof0 (or 1), c
	Input<Buffer<float>> steering_vectors{"steering_vectors", 3};
	// d, b, st, ch_tdof0 (or 1), c
	Output<Buffer<float>> adaptive_weights{"adaptive_weights", 5};
	void generate() {
		ComplexExpr c_0; // default constructor initializes both elements to 0.0f
		// TDOF*N_CHAN, doppler bin, training blocks, steering vectors
		Var ch_tdof0("ch_tdof0"), ch_tdof1("ch_tdof1"), ch_tdof2("ch_tdof2"), d("d"), b("b"), st("st"), c("c");
		ComplexFunc in_covariances("in_covariances");
		//in_covariances(d, b, ch_tdof0, ch_tdof1) = ComplexExpr(covariances(d, b, ch_tdof0, ch_tdof1, 0), covariances(d, b, ch_tdof0, ch_tdof1, 1));
		in_covariances(d, b, ch_tdof0, ch_tdof1) = ComplexExpr(covariances(0, ch_tdof1, ch_tdof0, b, d), covariances(1, ch_tdof1, ch_tdof0, b, d));
		ComplexFunc in_cholesky_factors("in_cholesky_factors");
		//in_cholesky_factors(d, b, ch_tdof0, ch_tdof1) = ComplexExpr(cholesky_factors(d, b, ch_tdof0, ch_tdof1, 0), cholesky_factors(d, b, ch_tdof0, ch_tdof1, 1));
		in_cholesky_factors(d, b, ch_tdof0, ch_tdof1) = ComplexExpr(cholesky_factors(0, ch_tdof1, ch_tdof0, b, d), cholesky_factors(1, ch_tdof1, ch_tdof0, b, d));
		ComplexFunc in_steering_vectors("in_steering_vectors");
		//in_steering_vectors(st, ch_tdof0) = ComplexExpr(steering_vectors(st, ch_tdof0, 0), steering_vectors(st, ch_tdof0, 1));
		in_steering_vectors(st, ch_tdof0) = ComplexExpr(steering_vectors(0, ch_tdof0, st), steering_vectors(1, ch_tdof0, st));

		// cholesky_factorization
		ComplexFunc cholesky("cholesky");
		cholesky(d, b, ch_tdof0, ch_tdof1) = in_cholesky_factors(d, b, ch_tdof0, ch_tdof1);

		// the rhs cholesky needs to be the complex conjugate
		RDom i(0, N_CHAN*TDOF, 0, N_CHAN*TDOF, 0, N_CHAN*TDOF);
		i.where(i.y > i.x);
		i.where(i.z >= i.y);
		cholesky(d, b, i.y, i.z) -= cholesky(d, b, i.x, i.z).re()*conj( cholesky(d, b, i.x, i.y) ).re()*1.0f/cholesky(d, b, i.x, i.y).re();
		cholesky.compute_root();
		// normalize each entry
		RDom j(0, N_CHAN*TDOF, 0, N_CHAN*TDOF);
		j.where(j.y >= j.x);
		cholesky(d, b, j.x, j.y) = ComplexExpr( Halide::sqrt( 1.0f / cholesky(d, b, j.x, j.x).re()), Halide::sqrt( 1.0f / cholesky(d, b, j.x, j.x).re()) );

		// copy upper righttriangle to lower left
		RDom k(0, N_CHAN*TDOF, 0, N_CHAN*TDOF);
		k.where(k.y > k.x);
		cholesky(d, b, k.y, k.x) = cholesky(d, b, k.x, k.y);
		cholesky.compute_root();

		// Forward_back_substitution
		ComplexFunc forward_back_substitution("forward_back_substitution");
		forward_back_substitution(d, b, st, ch_tdof2) = c_0;
		// forward substitution
		RDom l(0, N_CHAN*TDOF, 0, N_CHAN*TDOF);
		l.where(l.y < l.x);
		forward_back_substitution(d, b, st, l.x) += 
			(in_steering_vectors(st, l.x) - conj(cholesky(d, b, l.y, l.x))*forward_back_substitution(d, b, st, l.y))*
			1.0f / cholesky(d, b, l.x, l.x).re();
		forward_back_substitution.compute_root();
		// backsubstitition
		RDom m(N_CHAN*TDOF-1, 0, 0, N_CHAN*TDOF);
		m.where(m.y > m.x);
		forward_back_substitution(d, b, st, m.x) += 
			(forward_back_substitution(d, b, st, m.x) - cholesky(d, b, m.y, m.x)*forward_back_substitution(d, b, st, m.y))*
			1.0f / cholesky(d, b, m.x, m.x).re();

		adaptive_weights(c, ch_tdof2, st, b, d) = 0.0f;
		adaptive_weights(0, ch_tdof2, st, b, d) = forward_back_substitution(d, b, st, ch_tdof2).re();
		adaptive_weights(1, ch_tdof2, st, b, d) = forward_back_substitution(d, b, st, ch_tdof2).im();
	}
};

}  // namespace

HALIDE_REGISTER_GENERATOR(SYSTEMSOLVE, system_solve)
