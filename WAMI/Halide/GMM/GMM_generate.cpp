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
		// Functions that define operations to be done in-place on the input while the algorithm operates (leverages "update definitions", see halide iir_blur example)
		Func input("input");
		//input(f, x, y) = frame( clamp(f, 0, frame.height()), clamp(x, 0, frame.width()), clamp(y, 0, frame.channels() ) ); 
		input(f, x, y) = frame(f, x, y);
		Func input_mu("input_mu");
		//input_mu(x, y, m) = mu( clamp(x, 0, mu.height()), clamp(y, 0, mu.width()), clamp(m, 0, mu.channels() ) ); 
		input_mu(x, y, m) = mu(x, y, m);
		Func input_sigma("input_sigma");
		//input_sigma(x, y, m) = sigma( clamp(x, 0, sigma.height()), clamp(y, 0, sigma.width()), clamp(m, 0, sigma.channels() ) ); 
		input_sigma(x, y, m) = sigma(x, y, m);
		Func input_weights("input_weights");
		//input_weights(x, y, m) = weights( clamp(x, 0, weights.height()), clamp(y, 0, weights.width()), clamp(m, 0, weights.channels() ) ); 
		input_weights(x, y, m) = weights(x, y, m);

		// pixel weight update
		// remembers which distribution was correct
		Func stdev_thresh("stdev_thresh");
		stdev_thresh(f, x, y, m)  = Halide::abs( input(f, x, y) - input_mu(x, y, m)/input_sigma(x, y, m) );
		// current_match tells us which distribution matches the data
		// the reduction loops over all distributions of the current pixel in the current frame
		RDom k(0, WAMI_GMM_NUM_MODELS);
		Func current_match("current_match");
		current_match(f, x, y) = -1;
		current_match(f, x, y) = Halide::mux( stdev_thresh(f, x, y, k) < STDEV_THRESH, {current_match(f, x, y), k} );
		// update the parameter weights if any of our distributions match this pixel
		Func weight_update_0("weight_update_0"); // this is the "match" condition, we need to remember if is was ever exercised
		weight_update_0(x, y, m) += ALPHA * (1.0f - input_weights(x, y, m));
		Func weight_update_1("weight_update_1");
		weight_update_1(x, y, m) *= (1.0f - ALPHA); // this is the "not the match" condition, we update its weight with this
		Func weight_update("weight_update");
		weight_update(f, x, y, m) = Halide::mux( stdev_thresh(f, x, y, m) < STDEV_THRESH, {weight_update_1(x, y, m), weight_update_0(x, y, m)} );
		input_weights(x, y, k) = weight_update(f, x, y, k);

		// update mu and sigma for the distribution that matched
		// if none of the distributions matched the last pixel, we need to get rid of the least likely distribution, hence the mux conditions
		Func rho("rho");
		rho(f, x, y) = ALPHA * ONE_OVER_SQRT_TWO_PI * (1.0f / cast<float>(input_sigma(x, y, current_match(f, x, y))))* exp( -1.0f * pow( cast<float>(input(f, x, y)) - cast<float>(input_mu(x, y, current_match(f, x, y))), 2.0f) / 2.0f * pow( cast<float>(input_sigma(x, y, current_match(f, x, y))), 2.0f ) );
		Func update_distribution_mu("update_distribution_mu");
		update_distribution_mu(f, x, y) = Halide::mux( current_match(f, x, y) < 0 , { (1.0f - rho(f, x, y)) * input_mu(x, y, current_match(f, x, y)) + rho(f, x, y) * cast<float>(input(f, x, y)), cast<float>(input(f, x, y)) } );
		input_mu(x, y, Halide::mux( current_match(f, x, y) < 0, { current_match(f, x, y) , WAMI_GMM_NUM_MODELS-1 } )) = update_distribution_mu(f, x, y);
		Func update_distribution_sigma("update_distribution_sigma");
		update_distribution_sigma(f, x, y) = Halide::mux( current_match(f, x, y) < 0 , { sqrt( (1.0f - rho(f, x, y)) * pow( input_sigma(x, y, current_match(f, x, y)), 2.0f ) + rho(f, x, y) * pow( input(f, x, y) - input_mu(x, y, current_match(f, x, y)), 2.0f ) ), INIT_STDEV } );
		input_sigma(x, y, Halide::mux( current_match(f, x, y) < 0, { current_match(f, x, y) , WAMI_GMM_NUM_MODELS-1 } )) = update_distribution_sigma(f, x, y);
		Func update_distribution_weight("update_distribution_weight");
		update_distribution_weight(f, x, y) = Halide::mux( current_match(f, x, y) < 0 ,{ input_weights(x, y, WAMI_GMM_NUM_MODELS-1), INIT_WEIGHT });
		input_weights(x, y, Halide::mux( current_match(f, x, y) < 0, { current_match(f, x, y) , WAMI_GMM_NUM_MODELS-1 } )) = update_distribution_weight(f, x, y);

		// normalize all new weights
		RDom n(0, WAMI_GMM_NUM_MODELS);
		Func norm_sum("norm_sum");
		norm_sum(x, y) = 0.0f;
		norm_sum(x, y) += input_weights(x, y, n);
		Func normalized_weights("normalized_weights");
		normalized_weights(x, y, m) = input_weights(x, y, m) * 1.0f / norm_sum(x, y);
		input_weights(x, y, m) = normalized_weights(x, y, m);

		// sort new weights, if necessary
		//Func sort_distributions("sort_distributions");
		//sort_distributions(f, x, y) = 

		// finally, determine if the current pixel if foreground or background
		// this is done by determining whether the matching distribution exceeds the background threshold
		RDom o(0, WAMI_GMM_NUM_MODELS-1);
		Func accum_weights("accum_weights");
		accum_weights(x, y) = 0.0f;
		accum_weights(x, y) += input_weights(x, y, o);
		Func is_foreground("is_foreground");
		is_foreground(f, x, y) = cast<uint8_t>(Halide::mux( accum_weights(x, y) > BACKGROUND_THRESH , { 0 , 1 } ));
		foreground = is_foreground;
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(ChangeDetection, GMM)
