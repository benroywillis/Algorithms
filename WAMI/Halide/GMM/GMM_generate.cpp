#include "Halide.h"
#include <iostream>
#include <math.h>
#include "wami_params.h"

#define STDEV_THRESH 		2.5f
#define INIT_STDEV	 		80.0f
#define ALPHA 		 		0.01f
#define INIT_WEIGHT  		0.01f
#define BACKGROUND_THRESH   0.9f
#define ONE_OVER_SQRT_TWO_PI (1.0f / sqrt(2.0f * (float)M_PI))

namespace {

class ChangeDetection : public Halide::Generator<ChangeDetection> {
public:
	// input is 2 floating point first-derivatives of the image along both axes (x or y)
    Input<Buffer<float>> mu{"mu", 3};
    Input<Buffer<float>> sigma{"sigma", 3};
    Input<Buffer<float>> weights{"weights", 3};
    Input<Buffer<uint16_t>> frame{"frame", 3};
    Output<Buffer<uint8_t>> foreground{"foreground", 3};

    void generate() {
		// induction over the frames of the input
		Var f("f");
		// induction over the input image
		Var x("x"), y("y");
		// induction over the number of models in the input parameters
		Var m("m");
		// clamps the iterators to the boundaries of the input arrays
		Func input("input");
		input(f, x, y) = frame( clamp(f, 0, frame.height()), clamp(x, 0, frame.width()), clamp(y, 0, frame.channels() ) ); 
		Func input_mu("input_mu");
		input_mu(x, y, m) = mu( clamp(x, 0, mu.height()), clamp(y, 0, mu.width()), clamp(m, 0, mu.channels() ) ); 
		Func input_sigma("input_sigma");
		input_sigma(x, y, m) = sigma( clamp(x, 0, sigma.height()), clamp(y, 0, sigma.width()), clamp(m, 0, sigma.channels() ) ); 
		Func input_weights("input_weights");
		input_weights(x, y, m) = weights( clamp(x, 0, weights.height()), clamp(y, 0, weights.width()), clamp(m, 0, weights.channels() ) ); 

		// pixel weight update
		// remembers which distribution was correct
		Expr match = -1;
		// the reduction takes place over all distributions in the parameters
		RDom k(0, 5);
		Func stdev_thresh("stdev_thresh");
		stdev_thresh(f, x, y, m)  = Halide::abs( input(f, x, y) - input_mu(x, y, m)/input_sigma(x, y, m) );
		// updating match tells us which distribution matches the data
		Func match_update("match_update");
		match_update(f, x, y, m) = mux( stdev_thresh(f, x, y, m) < STDEV_THRESH, {m , match} );
		match = match_update(f, x, y, k);
		// update the parameter weights if any of our distributions match this pixel
		Func weight_update_0("weight_update_0"); // this is the "match" condition, we need to remember if is was ever exercised
		weight_update_0(x, y, m) += ALPHA * (1.0f - input_weights(x, y, m));
		Func weight_update_1("weight_update_1");
		weight_update_1(x, y, m) *= (1.0f - ALPHA);
		Func weight_update("weight_update");
		weight_update(f, x, y, m) = mux( stdev_thresh(f, x, y, m) < STDEV_THRESH, {weight_update_0(x, y, m), weight_update_1(x, y, m)} );
		weights(x, y, k) = weight_update(f, x, y, k);
/*
		// update mu and sigma for the distribution that matched, and sort the newfound weight in the parameter array
		// this will require sorting the new distributions in the array
		// if none of the distributions matched the last pixel, we need to get rid of the least likely distribution
		// the input parameters are sorted from most likely to least so this just becomes replacing the last entries
		// right now we don't know how to do this, so we just update it every time
		Func rho("rho");
		rho(f, x, y, m) = ALPHA * ONE_OVER_SQRT_TWO_PI * (1.0f / cast<float>(input_sigma(x, y, m)))* exp( -1.0f * pow( cast<float>(input(f, x, y)) - cast<float>(input_mu(x, y, m)), 2.0f) / 2.0f * pow( cast<float>(input_sigma(x, y, m)), 2.0f ) );
		Func update_distribution_mu("update_distribution_mu");
		update_distribution_mu(f, x, y, m) = mux( match < 0 , { (1.0f - rho(f, x, y, m)) * input_mu(x, y, m) + rho(f, x, y, m) * cast<float>(input(f, x, y)), cast<float>(input(f, x, y)) } );
		mu(x, y, clamp(0, 5, match)) = update_distribution_mu(f, x, y, match);
		Func update_distribution_sigma("update_distribution_sigma");
		update_distribution_sigma(f, x, y, match) = mux( match < 0 , { sqrt( (1.0f - rho(f, x, y, match)) * pow( input_sigma(x, y, match), 2.0f ) + rho(f, x, y, match) * pow( input(f, x, y) - input_mu(x, y, match), 2.0f ) ), INIT_STDEV } );
		sigma(x, y, clamp(0, 5, match)) = update_distribution_sigma(f, x, y, match);
		Func update_distribution_weight("update_distribution_weight");
		update_distribution_weight(f, x, y, match) = mux( match < 0 , { INIT_WEIGHT, input_weights(x, y, WAMI_GMM_NUM_MODELS-1) } );
		weights(x, y, clamp(0, 5, match)) = update_distribution_weight(f, x, y, match);
*/
		// normalize all new weights
		RDom n(0, 5);
		Func norm_sum("norm_sum");
		norm_sum(x, y) += input_weights(x, y, n);
		Func norm_weights("norm_weights");
		norm_weights(x, y) *= 1.0f / norm_sum(x, y);
		weights(x, y, k) = norm_weights(x, y);

		// resort the distributions in the model
		Expr sorted_position = 0;

		// finally, determine if the current pixel if foreground or background
		// this is done by determining whether the matching distribution exceeds the background threshold
		RDom o(0, 5);
		Func accum_weights("accum_weights");
		accum_weights(x, y) += input_weights(x, y, o);
		Func is_foreground("is_foreground");
		is_foreground(f, x, y) = mux( accum_weights(x, y) > BACKGROUND_THRESH , { 1 , 0 } );
		foreground(f, x, y) = cast<uint8_t>(is_foreground(f, x, y));
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ChangeDetection, GMM)
