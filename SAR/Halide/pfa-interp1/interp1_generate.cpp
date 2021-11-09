#include "Halide.h"
#include <math.h>

#include "sar_utils.h"
#include "sar_interp1.h"

#define fM_PI	(float)M_PI

namespace {

class Interpolation_1 : public Halide::Generator<Interpolation_1> {
public:
	// pulses, range, complex
    Input<Buffer<double>> data{"data", 3};
	// PFA
    Input<Buffer<float>> window{"window", 1};
	// pulses
    Input<Buffer<double>> input_start_coords{"input_start_coords", 1};
	// pulses
    Input<Buffer<double>> input_coord_spacing{"input_coord_spacing", 1};
	// pfa_nout_range
    Input<Buffer<double>> output_coords{"output_coords", 1};
	// pulses, range, complex
    Output<Buffer<double>> resampled{"resampled", 3};
	
    void generate() {
		Expr PFA_N_TSINC_POINTS_PER_SIDE = (T_PFA-1) / 2;
		// induction variables over pulses, range, PFA, pfa_nout_range, complex, and an auxiliary var for sinc() and other calculations
		Var p("p"), r("r"), pfa("pfa"), pnr("pnr"), c("c"), w("w"), x("x");

		Func scale_factor("scale_factor");
		scale_factor(p) = Halide::abs(output_coords(1) - output_coords(0)) * Halide::ConciseCasts::f64(1.0f) / input_coord_spacing(p);

		Func nearest_test("nearest_test");
		nearest_test(p ,r) = (output_coords(r) < input_start_coords(p)) || (output_coords(r) >= (input_start_coords(p) + (N_RANGE-1)*input_coord_spacing(p)));
		Func nearest_estimate("nearest_estimate");
		nearest_estimate(p, r) = Halide::mux( nearest_test(p, r), { cast<int>( ((output_coords(r) - input_start_coords(p)) * Halide::ConciseCasts::f64(1.0f) / input_coord_spacing(p)) + Halide::ConciseCasts::f64(0.5f) ), cast<int>(-1) } );

		Func round_condition("round_condition");
		round_condition(p, r) = Halide::abs(output_coords(r) - (input_start_coords(p) + (nearest_estimate(p, r)+1) * input_coord_spacing(p) )) < Halide::abs(output_coords(r) - (input_start_coords(p) + nearest_estimate(p, r) * input_coord_spacing(p) ));
		Func round_out_coord("found_out_coord");
		round_out_coord(p, r) = Halide::mux( round_condition(p, r), { nearest_estimate(p, r) , nearest_estimate(p, r) + 1} );

		Func window_min("window_min");
		window_min( p, r ) = clamp( round_out_coord(p, r) - PFA_N_TSINC_POINTS_PER_SIDE, 0, N_RANGE);
		Func window_max("window_max");
		window_max( p, r ) = clamp( round_out_coord(p, r) + PFA_N_TSINC_POINTS_PER_SIDE, 0, N_RANGE);
		Func window_offset("window_offset");
		window_offset( p, r ) = cast<int>(Halide::mux( round_out_coord(p, r) - PFA_N_TSINC_POINTS_PER_SIDE < 0, { 0 , PFA_N_TSINC_POINTS_PER_SIDE - round_out_coord(p, r) } ));
		
		Func sinc("sinc");
		sinc(x) = Halide::mux( Halide::ConciseCasts::f32(x) == Halide::ConciseCasts::f32(0.0f), { Halide::sin(Halide::ConciseCasts::f32(x) * fM_PI) / (Halide::ConciseCasts::f32(x)*fM_PI), Halide::ConciseCasts::f32(1.0f) } );
		
		//RDom k(window_min(p, r), window_max(p, r)+1);
		RDom k(0, N_RANGE-1);
		Func on_window("on_window");
		on_window(p, r, w) = Halide::mux( window_min(p, r) <= w <= window_max(p, r), { 0 , 1 } );
		Func sinc_arg("sinc_arg");
		sinc_arg(p, r, w) = Halide::cast<double>( output_coords(p) - (input_start_coords(p)+Halide::ConciseCasts::f64(w)*input_coord_spacing(p)) * Halide::ConciseCasts::f64(1.0f) / input_coord_spacing(p) );
		
		Func accum_window("accum_window");
		accum_window(p, r, w, c) = Halide::mux( on_window(p, r, w), { Halide::cast<double>(0.0f), 
			Halide::mux( Halide::cast<float>(sinc_arg(p, r, w)) == Halide::cast<float>(0.0f), { Halide::sin(Halide::cast<float>(sinc_arg(p, r, w)) * fM_PI) / 
			(Halide::cast<float>(sinc_arg(p, r, w))*fM_PI), Halide::cast<float>(1.0f) } ) * window( clamp(window_offset(p, r) + w - window_min(p, r), 0, T_PFA-1) ) * data(p, w, c) } );
		
		Func valid_nearest("valid_nearest");
		valid_nearest(p, r, w, c) = Halide::mux( nearest_estimate(p, r) > 0, { Halide::ConciseCasts::f64(0.0f), accum_window(p, r, w, c) } );
		resampled(p, r, c) = Halide::cast<double>(0);
		resampled(p, r, c) = mux( c, { valid_nearest(p, r, k, 0), valid_nearest(p, r, k, 1) } );
	}
};

}  // namespace

HALIDE_REGISTER_GENERATOR(Interpolation_1, interp1)
