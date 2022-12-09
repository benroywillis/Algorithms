
#include "Halide.h"

using namespace Halide;

#include <iostream>
#include <limits>
#include <memory>
#include <cfloat>
#include <vector>
#include <sys/time.h>

char * func_name(const char *s, int line)
{
    static int k = 0;
    char *ss = (char*) malloc(128);
    sprintf(ss, "%s_%d_%d", s, line, k++);
    return ss;
}

#define DECL_FUNC(name) Func name(func_name(#name, __LINE__));

class SIFT : public Generator<SIFT> {
public:
    #define SIGMA 1.6f
    Input<Buffer<float>> input{"input", 3};
    GeneratorParam<int> octaves{"octaves", 1};
    GeneratorParam<int> intervals{"intervals", 1};
    Input<float> curv_thr{"curv_thr", 1};
    Input<float> contr_thr{"contr_thr", 1};
    Output<Buffer<uint8_t>> out{"out", 2};

    void generate() 
    {
		// octaves, intervals, columns, rows, colors
		Var o("o"), i("i"), x("x"), y("y"), c("c");

        // convert to grayscale
        DECL_FUNC(gray);
        gray(x, y) = 0.299f * input(x, y, 0) + 0.587f * input(x, y, 1) + 0.114f * input(x, y, 2);
        // bound the input to the parameters of the image
        DECL_FUNC(clamped);
        clamped(x, y) = gray(clamp(x, 0, input.width()  - 1), clamp(y, 0, input.height() - 1));

        // calculate gaussian pyramids, which requires downsampling (in pixel space) and blurring (in color space)
        DECL_FUNC(downx);
        DECL_FUNC(downy);
        DECL_FUNC(downsample);
        downx(x, y) = (clamped(2*x-1, y) + 3.0f * (clamped(2*x, y) + clamped(2*x+1, y)) + clamped(2*x+2, y)) / 8.0f;
        downy(x, y) = (downx(x, 2*y-1) + 3.0f * (downx(x, 2*y) + downx(x, 2*y+1)) + downx(x, 2*y+2)) / 8.0f;
        downsample(x, y) = downy(x, y);

        DECL_FUNC(p);
        DECL_FUNC(sigma);
        DECL_FUNC(gaussian);
        p(i) = Halide::pow( 2.0f, 1.0f / Halide::cast<float>(i) );
        sigma(i) = Halide::sqrt( Halide::pow( p(i), i - 1 )*p(i) * Halide::pow( p(i), i - 1 )*p(i) - Halide::pow( p(i), i - 1 ) * SIGMA * Halide::pow( p(i), i - 1 ) * SIGMA );
        gaussian(x, i) = Halide::exp( -(x/sigma(i))*(x/sigma(i))*0.5f );

        //DECL_FUNC(radius)
        DECL_FUNC(normalized);
        DECL_FUNC(blurx);
        DECL_FUNC(blury);
        DECL_FUNC(blur);
        // reduction over window
        // this section does not reflect the original version of the algorithm because the bounds of the reduction cannot be dependent on a func
        // the original algorithm had this reduction variable over the current interval, but this code just fixes the reduction to the GeneratorParam "interval"
        Expr radius = 3.0f*Halide::sqrt( Halide::pow( Halide::pow( 2.0f, 1.0f / Halide::cast<float>(intervals) ), intervals - 1 )*Halide::pow( 2.0f, 1.0f / Halide::cast<float>(intervals) ) * Halide::pow( Halide::pow( 2.0f, 1.0f / Halide::cast<float>(intervals) ), intervals - 1 )*Halide::pow( 2.0f, 1.0f / Halide::cast<float>(intervals) ) - Halide::pow( Halide::pow( 2.0f, 1.0f / Halide::cast<float>(intervals) ), intervals - 1 ) * SIGMA * Halide::pow( Halide::pow( 2.0f, 1.0f / Halide::cast<float>(intervals) ), intervals - 1 ) * SIGMA )  + 1.0f;
        {
          RDom r(Halide::cast<int>(-radius), Halide::cast<int>(2*radius + 1));
          normalized(x, i) = gaussian(x, i) / sum(gaussian(r, i));
          blurx(x, y) += clamped(x+r, y) * normalized(x, r);
          blury(x, y) += blurx(x, y+r) * normalized(x, r);
        }
        blur(x, y) = blury(x, y);
        
		DECL_FUNC(gauss_pyr);
        gauss_pyr(o, i, x, y) = Halide::select( (o == 0) && (i == 0), clamped(x, y),
                                                (i == 0), downsample( x, y ),
                                                blur( x, y ) );

        // Difference of Gaussians
        DECL_FUNC(dog_pyr);
        dog_pyr(o, i, x, y) = gauss_pyr(o, i+1, x, y) - gauss_pyr(o, i, x, y);

        Expr dmin, dmax;
        {
            RDom r(-1,2,-1,2);
            dmax = max( dog_pyr(o, i+1, x+r.x, y+r.y), 
                	    max( dog_pyr(o, i, x+r.x, y+r.y), dog_pyr(o, i-1, x+r.x, y+r.y) )
              );
            dmin = min( dog_pyr(o, i+1, x+r.x, y+r.y), 
                		min( dog_pyr(o, i, x+r.x, y+r.y), dog_pyr(o, i-1, x+r.x, y+r.y) )
              );
        }
        DECL_FUNC(dog_max);
        dog_max(o, i, x, y) = -FLT_MAX;
        dog_max(o, i, x, y) = max(dog_max(o, i, x, y), dmax);

        DECL_FUNC(dog_min);
        dog_min(o, i, x, y) = FLT_MAX;
        dog_min(o, i, x, y) = min(dog_min(o, i, x, y), dmin);

        Expr prelim_contr_thr = 0.5f * contr_thr / Halide::cast<float>(intervals);
        Expr v = dog_pyr(o, i, x, y);
        DECL_FUNC(is_extremum);
        is_extremum(o, i, x, y) = ( (abs(v) > prelim_contr_thr) && 
							  		( (v <= 0.0f && v == dog_min(o, i, x, y)) || 
									  (v >  0.0f && v == dog_max(o, i, x, y)) 
							  		)
						   		  );

		// b. Hessian on 2x2 windows
        DECL_FUNC(dx);
        dx(o, i, x, y) = (dog_pyr(o, i, x+1, y) - dog_pyr(o, i, x-1, y)) / 2.0f;

        DECL_FUNC(dy);
        dy(o, i, x, y) = (dog_pyr(o, i, x, y+1) - dog_pyr(o, i, x, y-1)) / 2.0f;

        DECL_FUNC(ds);
        ds(o, i, x, y) = (dog_pyr(o, i+1, x, y) - dog_pyr(o, i-1, x, y)) / 2.0f;

        #define HESSIAN_XX 0
        #define HESSIAN_YY 1
        #define HESSIAN_SS 2
        #define HESSIAN_XY 3
        #define HESSIAN_XS 4
        #define HESSIAN_YS 5

        DECL_FUNC(hessian_xx);
		hessian_xx(o, i, x, y) = dog_pyr(o, i  , x+1, y  ) + dog_pyr(o, i  , x-1, y  ) - 2.0f * v;
        DECL_FUNC(hessian_yy);
		hessian_yy(o, i, x, y) = dog_pyr(o, i  , x  , y+1) + dog_pyr(o, i  , x  , y-1) - 2.0f * v;
        DECL_FUNC(hessian_ss);
		hessian_ss(o, i, x, y) = dog_pyr(o, i+1, x  , y  ) + dog_pyr(o, i-1, x  , y  ) - 2.0f * v;
        DECL_FUNC(hessian_xy);
		hessian_xy(o, i, x, y) = ( dog_pyr(o, i  , x+1, y+1) - dog_pyr(o, i  , x-1, y+1) - 
					 			   dog_pyr(o, i  , x+1, y-1) + dog_pyr(o, i  , x-1, y-1) ) / 4.0f;
        DECL_FUNC(hessian_xs);
		hessian_xs(o, i, x, y) = ( dog_pyr(o, i+1, x+1, y  ) - dog_pyr(o, i+1, x-1, y  ) - 
					 			   dog_pyr(o, i-1, x+1, y  ) + dog_pyr(o, i-1, x-1, y  ) ) / 4.0f;
        DECL_FUNC(hessian_ys);
		hessian_ys(o, i, x, y) = ( dog_pyr(o, i+1, x  , y+1) - dog_pyr(o, i+1, x  , y-1) - 
					 			   dog_pyr(o, i-1, x  , y+1) + dog_pyr(o, i-1, x  , y-1) ) / 4.0f;
	
        DECL_FUNC(pc_det);
        pc_det(o, i, x, y) = hessian_xy(o, i, x, y) * hessian_yy(o, i, x, y) - 2.0f * hessian_xy(o, i, x, y);

        DECL_FUNC(pc_tr);
        pc_tr(o, i, x, y)  = hessian_xx(o, i, x, y) + hessian_yy(o, i, x, y);

        DECL_FUNC(invdet);
        invdet(o, i, x, y) = 1.0f/(   ( hessian_xx(o, i, x, y) * (hessian_yy(o, i, x, y) * hessian_ss(o, i, x, y) - 
							   		    hessian_ys(o, i, x, y) *  hessian_ys(o, i, x, y) ) ) 
					       			- ( hessian_xy(o, i, x, y) * (hessian_xy(o, i, x, y) * hessian_ss(o, i, x, y) - 
						       			hessian_ys(o, i, x, y) *  hessian_xs(o, i, x, y) ) )
                           			+ ( hessian_xs(o, i, x, y) * (hessian_xy(o, i, x, y) * hessian_ys(o, i, x, y) - 
						       			hessian_yy(o, i, x, y) *  hessian_xs(o, i, x, y) ) )
						   		  );

		DECL_FUNC(inv_xx);
		inv_xx(o, i, x, y) = invdet(o, i, x, y) * (hessian_yy(o, i, x, y) * hessian_ss(o, i, x, y) - hessian_ys(o, i, x, y) * hessian_ys(o, i, x, y));
		DECL_FUNC(inv_yy);
		inv_yy(o, i, x, y) = (invdet(o, i, x, y)) * (hessian_xx(o, i, x, y) * hessian_ss(o, i, x, y) - hessian_xs(o, i, x, y) * hessian_xs(o, i, x, y));
		DECL_FUNC(inv_ss);
		inv_ss(o, i, x, y) = (invdet(o, i, x, y)) * (hessian_xx(o, i, x, y) * hessian_yy(o, i, x, y) - hessian_xy(o, i, x, y) * hessian_xy(o, i, x, y));
		DECL_FUNC(inv_xy);
		inv_xy(o, i, x, y) = (invdet(o, i, x, y)) * (hessian_xs(o, i, x, y) * hessian_ys(o, i, x, y) - hessian_xy(o, i, x, y) * hessian_ss(o, i, x, y));
		DECL_FUNC(inv_xs);
		inv_xs(o, i, x, y) = (invdet(o, i, x, y)) * (hessian_xy(o, i, x, y) * hessian_ys(o, i, x, y) - hessian_xs(o, i, x, y) * hessian_yy(o, i, x, y));
		DECL_FUNC(inv_ys);
		inv_ys(o, i, x, y) = (invdet(o, i, x, y)) * (hessian_xy(o, i, x, y) * hessian_xs(o, i, x, y) - hessian_xx(o, i, x, y) * hessian_ys(o, i, x, y));

		DECL_FUNC(interp_xx);
		interp_xx(o, i, x, y) = inv_xx(o, i, x, y) * dx(o, i, x, y) + inv_xy(o, i, x,y) * dy(o, i, x, y) + inv_xs(o, i, x, y) * ds(o, i, x, y);
		DECL_FUNC(interp_xy);
		interp_xy(o, i, x, y) = inv_xy(o, i, x, y) * dx(o, i, x, y) + inv_yy(o, i, x,y) * dy(o, i, x, y) + inv_ys(o, i, x, y) * ds(o, i, x, y);
		DECL_FUNC(interp_xs);
		interp_xs(o, i, x, y) = inv_xs(o, i, x, y) * dx(o, i, x, y) + inv_ys(o, i, x,y) * dy(o, i, x, y) + inv_ss(o, i, x, y) * ds(o, i, x, y);

        DECL_FUNC(interp_contr);
        interp_contr(o, i, x, y) = interp_xx(o, i, x, y) * dx(o, i, x, y) + interp_xy(o, i, x, y) * dy(o, i, x, y) + interp_xs(o, i, x, y) * ds(o, i, x, y);

        DECL_FUNC(key);
        key(o, i, x, y) = Halide::cast<bool>(is_extremum(o, i, x, y)) &&
          				  (pc_det(o, i, x, y) > 0.0f) &&
                       	  ( (pc_tr(o, i, x, y) * pc_tr(o, i, x, y) / pc_det(o, i, x, y)) < 
						    (( curv_thr + 1.0f )*( curv_thr + 1.0f ) / curv_thr) ) &&
                      	  (abs(interp_contr(o, i, x, y)) > (contr_thr / intervals)) &&
                      	  (dx(o, i, x, y) < 1.0f) &&
                      	  (dy(o, i, x, y) < 1.0f) &&
                      	  (ds(o, i, x, y) < 1.0f); 
		
        // combine the results across octaves and intervals
		RDom over(0, octaves, 1, intervals+1);
		DECL_FUNC(expr_comb);
		expr_comb(x, y) = Halide::cast<bool>(0);
        expr_comb(x, y) = select( ((x % (1 << over.x)) == 0) && ((y % (1 << over.x)) == 0), (key(over.x, over.y, x / (1 << over.x), y / (1 << over.x)) || expr_comb(x, y)), expr_comb(x, y));

        Func comb_ext;
        comb_ext(x,y) = 255 * cast<uint8_t>(expr_comb(x, y));
        out(x, y) = comb_ext(x, y);

		// Schedule
		// gaussian schedule allows halide to stop inlining this expensive calculation everywhere
		sigma.compute_at(gaussian, x); // this schedule barely makes a difference but increases variance of performance
		gaussian.compute_root();// adding these parameters makes 16-thread performance worse -> .parallel(x);//.vectorize(x, 4);
		// blur operation is vectorizable along the columns
		blur.compute_root().parallel(y).vectorize(x, 4); 
		// this line makes performance a lot worse -> gauss_pyr.compute_root().parallel(o).parallel(i);
		// this makes barely any difference in performance -> dog_pyr.compute_at(out, y).parallel(o).parallel(i).vectorize(x, 4);
        dog_min.parallel(y).vectorize(x, 4); // this and the one below make performance more consistently good, but not better
        dog_max.parallel(y).vectorize(x, 4);
		hessian_xx.compute_root().parallel(y).vectorize(x, 4);
		hessian_yy.compute_root().parallel(y).vectorize(x, 4);
		hessian_ss.compute_root().parallel(y).vectorize(x, 4);
		hessian_xy.compute_root().parallel(y).vectorize(x, 4);
		hessian_xs.compute_root().parallel(y).vectorize(x, 4);
		hessian_ys.compute_root().parallel(y).vectorize(x, 4);
		pc_det.compute_root().parallel(y).vectorize(x, 4);
		pc_tr.compute_root().parallel(y).vectorize(x, 4);
		invdet.compute_root().parallel(y).vectorize(x, 4);
		inv_xx.compute_root().parallel(y).vectorize(x, 4);
		inv_yy.compute_root().parallel(y).vectorize(x, 4);
		inv_ss.compute_root().parallel(y).vectorize(x, 4);
		inv_xy.compute_root().parallel(y).vectorize(x, 4);
		inv_xs.compute_root().parallel(y).vectorize(x, 4);
		inv_ys.compute_root().parallel(y).vectorize(x, 4);
		interp_xx.compute_root().parallel(y).vectorize(x, 4);
		interp_xy.compute_root().parallel(y).vectorize(x, 4);
		interp_xs.compute_root().parallel(y).vectorize(x, 4);
		interp_contr.compute_root().parallel(y).vectorize(x, 4);
		key.compute_root().parallel(y).vectorize(x, 4);
		comb_ext.compute_root().parallel(y).vectorize(x, 4);
		//expr_comb.compute_root();

    }
};

HALIDE_REGISTER_GENERATOR(SIFT, sift);
