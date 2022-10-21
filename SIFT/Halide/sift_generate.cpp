
#include "Halide.h"

using namespace Halide;

#include <iostream>
#include <limits>
#include <memory>
#include <cfloat>
#include <vector>
#include <sys/time.h>

Var o("o"), i("i"), x("x"), y("y"), c("c"), k("k"), fi("fi");

#define POW2(x) ((x)*(x))

#define NTRIES 10

char * func_name(const char *s, int line)
{
  static int k = 0;
  char *ss = (char*) malloc(128);
  sprintf(ss, "%s_%d_%d", s, line, k++);
  return ss;
}

#define DECL_FUNC(name) Func name(func_name(#name, __LINE__));

double now() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    static bool first_call = true;
    static time_t first_sec = 0;
    if (first_call) {
        first_call = false;
        first_sec = tv.tv_sec;
    }
    assert(tv.tv_sec >= first_sec);
    return (tv.tv_sec - first_sec) + (tv.tv_usec / 1000000.0);
}

struct Stats
{
  float min;
  float max;
  float elapsed[NTRIES];

  Stats(){
    min =  FLT_MAX;
    max = -FLT_MAX;
    for (int k=0; k<NTRIES; k++) elapsed[k] = FLT_MAX;
  }
};

#define TIME_START(st)                                \
{                                                     \
  double start = now();                               \
  {

#define TIME_END(st, i)                               \
  }                                                   \
  double end   = now();                               \
                                                      \
  st.elapsed[i] = end - start;                        \
  if (st.elapsed[i] < st.min) st.min = st.elapsed[i]; \
  if (st.elapsed[i] > st.max) st.max = st.elapsed[i]; \
}

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
    downx(x, y) = (clamped(2*x-1, y) + 3.0f * (gray(2*x, y) + gray(2*x+1, y)) + gray(2*x+2, y)) / 8.0f;
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
    //radius(i) = 3.0f*Halide::sqrt( Halide::pow( p(i), i - 1 )*p(i) * Halide::pow( p(i), i - 1 )*p(i) - Halide::pow( p(i), i - 1 ) * SIGMA * Halide::pow( p(i), i - 1 ) * SIGMA )  + 1.0f;
    Expr radius = 3.0f*Halide::sqrt( Halide::pow( Halide::pow( 2.0f, 1.0f / Halide::cast<float>(intervals) ), intervals - 1 )*Halide::pow( 2.0f, 1.0f / Halide::cast<float>(intervals) ) * Halide::pow( Halide::pow( 2.0f, 1.0f / Halide::cast<float>(intervals) ), intervals - 1 )*Halide::pow( 2.0f, 1.0f / Halide::cast<float>(intervals) ) - Halide::pow( Halide::pow( 2.0f, 1.0f / Halide::cast<float>(intervals) ), intervals - 1 ) * SIGMA * Halide::pow( Halide::pow( 2.0f, 1.0f / Halide::cast<float>(intervals) ), intervals - 1 ) * SIGMA )  + 1.0f;
    {
      RDom r(Halide::cast<int>(-radius), Halide::cast<int>(2*radius + 1));
      normalized(x, i) = gaussian(x, i) / sum(gaussian(r, i));
      blurx(x, y) += gray(x+r, y) * normalized(x, r);
      blury(x, y) += blurx(x, y+r) * normalized(x, r);
    }
    blur(x, y) = blury(x, y);

    DECL_FUNC(gauss_pyr);
    gauss_pyr(o, i, x, y) = 0.0f;
    gauss_pyr(0, 0, x, y) = clamped(x, y);
    gauss_pyr(o, i, x, y) = Halide::select( (o == 0) && (i == 0), clamped(x, y),
                                            (i == 0), downsample( x, y ),
                                            blur( x, y ) );
    
    // Difference of Gaussians
    DECL_FUNC(dog_pyr);
    dog_pyr(o, i, x, y) = gauss_pyr(o, i+1, x, y) - gauss_pyr(o, i, x, y);

    // 1. Extrema detection
    /*Expr v = dog_pyr(o, i, x, y);

    RDom r(-1,2,-1,2);

    Expr dmax = max(dog_pyr(o, i, x+r.x,y+r.y), max(dog_pyr(o, i, x+r.x,y+r.y), dog_pyr(o, i, x+r.x,y+r.y)));

    DECL_FUNC(dog_max);
    dog_max(o, i, x,y) = -FLT_MAX;
    dog_max(o, i, x,y) = max(dog_max(o, i, x,y), dmax);

    Expr dmin = min(dog_pyr(o, i, x+r.x,y+r.y), min(dog_pyr(o, i, x+r.x,y+r.y), dog_pyr(o, i, x+r.x,y+r.y)));

    DECL_FUNC(dog_min);
    dog_min(o, i, x, y) = FLT_MAX;
    dog_min(o, i, x, y) = min(dog_min(o, i, x, y), dmin);

    DECL_FUNC(is_extremum);
    Expr prelim_contr_thr = 0.5f * contr_thr / Halide::cast<float>(intervals);
    is_extremum(o, i, x, y) = ( ((abs(v) > prelim_contr_thr) && (v <= 0.0f) && (v == dog_min(o, i, x,y))) || (v >  0.0f && v == dog_max(o, i, x,y)) );*/
    RDom over(0, octaves, 1, intervals+1);

    Expr prelim_contr_thr = 0.5f * contr_thr / Halide::cast<float>(intervals);
    Expr v = dog_pyr(over.x, over.y, x,y);

    DECL_FUNC(dog_max);
    Expr dmin, dmax;
    {
      RDom r(-1,2,-1,2);
      dmax = max(dog_pyr(over.x, over.y+1,x+r.x,y+r.y), max(dog_pyr(over.x, over.y, x+r.x,y+r.y), dog_pyr(over.x, over.y-1, x+r.x,y+r.y)));

      dog_max(x,y) = -FLT_MAX;
      dog_max(x,y) = max(dog_max(x,y), dmax);

      dmin = min(dog_pyr(over.x, over.y+1,x+r.x,y+r.y), min(dog_pyr(over.x, over.y, x+r.x,y+r.y), dog_pyr(over.x, over.y-1, x+r.x,y+r.y)));
    }

    DECL_FUNC(dog_min);
    dog_min(x,y) = FLT_MAX;
    dog_min(x,y) = min(dog_min(x,y), dmin);

    dog_min.parallel(y).vectorize(x, 4);
    dog_max.parallel(y).vectorize(x, 4);

    DECL_FUNC(is_extremum);
    is_extremum(x,y) = ((abs(v) > prelim_contr_thr) && ((v <= 0.0f && v == dog_min(x,y)) || (v >  0.0f && v == dog_max(x,y))));

    // b. Hessian on 2x2 windows
    DECL_FUNC(dx);
    dx(x,y) = (dog_pyr(over.x, over.y, x+1, y) - dog_pyr(over.x, over.y, x-1, y)) / 2.0f;

    DECL_FUNC(dy);
    dy(x,y) = (dog_pyr(over.x, over.y, x, y+1) - dog_pyr(over.x, over.y, x, y-1)) / 2.0f;

    DECL_FUNC(ds);
    ds(x,y) = (dog_pyr(over.x, over.y+1,x, y) - dog_pyr(over.x, over.y-1,x, y)) / 2.0f;

    Expr dxx = dog_pyr(over.x, over.y, x+1, y  ) + dog_pyr(over.x, over.y, x-1, y  ) - 2.0f * v;
    Expr dyy = dog_pyr(over.x, over.y, x,   y+1) + dog_pyr(over.x, over.y, x,   y-1) - 2.0f * v;
    Expr dss = dog_pyr(over.x, over.y+1,x,   y  ) + dog_pyr(over.x, over.y-1,x,   y  ) - 2.0f * v;
    Expr dxy = ( dog_pyr(over.x, over.y, x+1, y+1) - dog_pyr(over.x, over.y, x-1, y+1) - dog_pyr(over.x, over.y, x+1, y-1) + dog_pyr(over.x, over.y, x-1, y-1) ) / 4.0f;
    Expr dxs = ( dog_pyr(over.x, over.y+1,x+1, y  ) - dog_pyr(over.x, over.y+1,x-1, y  ) - dog_pyr(over.x, over.y-1,x+1, y  ) + dog_pyr(over.x, over.y-1,x-1, y  ) ) / 4.0f;
    Expr dys = ( dog_pyr(over.x, over.y+1,x,   y+1) - dog_pyr(over.x, over.y+1,x,   y-1) - dog_pyr(over.x, over.y-1,x,   y+1) + dog_pyr(over.x, over.y-1,x,   y-1) ) / 4.0f;

    #define HESSIAN_XX 0
    #define HESSIAN_YY 1
    #define HESSIAN_SS 2
    #define HESSIAN_XY 3
    #define HESSIAN_XS 4
    #define HESSIAN_YS 5

    DECL_FUNC(hessian);
    hessian(c,x,y) = select(c == HESSIAN_XX, dxx,
                    select(c == HESSIAN_YY, dyy,
                    select(c == HESSIAN_SS, dss,
                    select(c == HESSIAN_XY, dxy,
                    select(c == HESSIAN_XS, dxs,
                                            dys)))));

    DECL_FUNC(pc_det);
    pc_det(x,y) = hessian(HESSIAN_XX,x,y) * hessian(HESSIAN_YY,x,y) - 2.0f * hessian(HESSIAN_XY,x,y);

    DECL_FUNC(pc_tr);
    pc_tr(x,y)  = hessian(HESSIAN_XX,x,y) + hessian(HESSIAN_YY,x,y);

    DECL_FUNC(invdet);
    invdet(x,y) = 1.0f/(  ( hessian(HESSIAN_XX,x,y) * (hessian(HESSIAN_YY,x,y) * hessian(HESSIAN_SS,x,y) - hessian(HESSIAN_YS,x,y) * hessian(HESSIAN_YS,x,y)) )
                        - ( hessian(HESSIAN_XY,x,y) * (hessian(HESSIAN_XY,x,y) * hessian(HESSIAN_SS,x,y) - hessian(HESSIAN_YS,x,y) * hessian(HESSIAN_XS,x,y)) )
                        + ( hessian(HESSIAN_XS,x,y) * (hessian(HESSIAN_XY,x,y) * hessian(HESSIAN_YS,x,y) - hessian(HESSIAN_YY,x,y) * hessian(HESSIAN_XS,x,y)) ));

    DECL_FUNC(inv);
    inv(c,x,y) = select(c == HESSIAN_XX, (invdet(x,y)) * (hessian(HESSIAN_YY,x,y) * hessian(HESSIAN_SS,x,y) - hessian(HESSIAN_YS,x,y) * hessian(HESSIAN_YS,x,y)),
                select(c == HESSIAN_YY, (invdet(x,y)) * (hessian(HESSIAN_XX,x,y) * hessian(HESSIAN_SS,x,y) - hessian(HESSIAN_XS,x,y) * hessian(HESSIAN_XS,x,y)),
                select(c == HESSIAN_SS, (invdet(x,y)) * (hessian(HESSIAN_XX,x,y) * hessian(HESSIAN_YY,x,y) - hessian(HESSIAN_XY,x,y) * hessian(HESSIAN_XY,x,y)),
                select(c == HESSIAN_XY, (invdet(x,y)) * (hessian(HESSIAN_XS,x,y) * hessian(HESSIAN_YS,x,y) - hessian(HESSIAN_XY,x,y) * hessian(HESSIAN_SS,x,y)),
                select(c == HESSIAN_XS, (invdet(x,y)) * (hessian(HESSIAN_XY,x,y) * hessian(HESSIAN_YS,x,y) - hessian(HESSIAN_XS,x,y) * hessian(HESSIAN_YY,x,y)),
                                        (invdet(x,y)) * (hessian(HESSIAN_XY,x,y) * hessian(HESSIAN_XS,x,y) - hessian(HESSIAN_XX,x,y) * hessian(HESSIAN_YS,x,y)))))));

    DECL_FUNC(interp);
    interp(c,x,y) = select(c == 0, inv(HESSIAN_XX, x,y) * dx(x,y) + inv(HESSIAN_XY, x,y) * dy(x,y) + inv(HESSIAN_XS, x,y) * ds(x,y),
                    select(c == 1, inv(HESSIAN_XY, x,y) * dx(x,y) + inv(HESSIAN_YY, x,y) * dy(x,y) + inv(HESSIAN_YS, x,y) * ds(x,y),
                                  inv(HESSIAN_XS, x,y) * dx(x,y) + inv(HESSIAN_YS, x,y) * dy(x,y) + inv(HESSIAN_SS, x,y) * ds(x,y)));

    DECL_FUNC(interp_contr);
    interp_contr(x,y) = interp(0,x,y) * dx(x,y) + interp(1,x,y) * dy(x,y) + interp(2,x,y) * ds(x,y);

    Expr is_valid = is_extremum(x,y) &&
                    pc_det(x,y) > 0.0f &&
                    (pc_tr(x,y) * pc_tr(x,y) / pc_det(x,y) < ( curv_thr + 1.0f )*( curv_thr + 1.0f ) / curv_thr) &&
                    abs(interp_contr(x,y)) > contr_thr / intervals &&
                    dx(x,y) < 1.0f &&
                    dy(x,y) < 1.0f &&
                    ds(x,y) < 1.0f;
    DECL_FUNC(key);
    key(over.x, over.y-1,x,y) = is_valid;
    // combine the results across octaves and intervals
    Expr expr_comb = false;
    expr_comb = select(x % (1 << over.x) == 0 && y % (1 << over.x) == 0, (key(over.x, over.y, x / (1 << over.x), y / (1 << over.x)) || expr_comb), expr_comb);

    Func comb_ext;
    comb_ext(x,y) = 255 * cast<uint8_t>(expr_comb);
    comb_ext.parallel(y).vectorize(x, 4);
    out = comb_ext;
    out(x, y) = Halide::cast<uint8_t>(0);

    /*

    Expr prelim_contr_thr = 0.5f * contr_thr / Halide::cast<float>(intervals);*/
    //Func key("key");
    /*RDom over(0, octaves, 1, intervals+1);

    Expr prelim_contr_thr = 0.5f * contr_thr / Halide::cast<float>(intervals);
    Expr v = dog_pyr(over.x, over.y, x,y);

    RDom r(-1,2,-1,2);
    Expr dmax = max(dog_pyr(over.x, over.y+1,x+r.x,y+r.y), max(dog_pyr(over.x, over.y, x+r.x,y+r.y), dog_pyr(over.x, over.y-1, x+r.x,y+r.y)));

    DECL_FUNC(dog_max);
    dog_max(x,y) = -FLT_MAX;
    dog_max(x,y) = max(dog_max(x,y), dmax);

    Expr dmin = min(dog_pyr(over.x, over.y+1,x+r.x,y+r.y), min(dog_pyr(over.x, over.y, x+r.x,y+r.y), dog_pyr(over.x, over.y-1, x+r.x,y+r.y)));

    DECL_FUNC(dog_min);
    dog_min(x,y) = FLT_MAX;
    dog_min(x,y) = min(dog_min(x,y), dmin);

    dog_min.parallel(y).vectorize(x, 4);
    dog_max.parallel(y).vectorize(x, 4);

    DECL_FUNC(is_extremum);
    is_extremum(x,y) = ((abs(v) > prelim_contr_thr) && ((v <= 0.0f && v == dog_min(x,y)) || (v >  0.0f && v == dog_max(x,y))));

    // 2nd part

    DECL_FUNC(dx);
    dx(x,y) = (dog_pyr(over.x, over.y, x+1, y) - dog_pyr(over.x, over.y, x-1, y)) / 2.0f;

    DECL_FUNC(dy);
    dy(x,y) = (dog_pyr(over.x, over.y, x, y+1) - dog_pyr(over.x, over.y, x, y-1)) / 2.0f;

    DECL_FUNC(ds);
    ds(x,y) = (dog_pyr(over.x, over.y+1,x, y) - dog_pyr(over.x, over.y-1,x, y)) / 2.0f;

    Expr dxx = dog_pyr(over.x, over.y, x+1, y  ) + dog_pyr(over.x, over.y, x-1, y  ) - 2.0f * v;
    Expr dyy = dog_pyr(over.x, over.y, x,   y+1) + dog_pyr(over.x, over.y, x,   y-1) - 2.0f * v;
    Expr dss = dog_pyr(over.x, over.y+1,x,   y  ) + dog_pyr(over.x, over.y-1,x,   y  ) - 2.0f * v;
    Expr dxy = ( dog_pyr(over.x, over.y, x+1, y+1) - dog_pyr(over.x, over.y, x-1, y+1) - dog_pyr(over.x, over.y, x+1, y-1) + dog_pyr(over.x, over.y, x-1, y-1) ) / 4.0f;
    Expr dxs = ( dog_pyr(over.x, over.y+1,x+1, y  ) - dog_pyr(over.x, over.y+1,x-1, y  ) - dog_pyr(over.x, over.y-1,x+1, y  ) + dog_pyr(over.x, over.y-1,x-1, y  ) ) / 4.0f;
    Expr dys = ( dog_pyr(over.x, over.y+1,x,   y+1) - dog_pyr(over.x, over.y+1,x,   y-1) - dog_pyr(over.x, over.y-1,x,   y+1) + dog_pyr(over.x, over.y-1,x,   y-1) ) / 4.0f;

    #define HESSIAN_XX 0
    #define HESSIAN_YY 1
    #define HESSIAN_SS 2
    #define HESSIAN_XY 3
    #define HESSIAN_XS 4
    #define HESSIAN_YS 5

    DECL_FUNC(hessian);
    hessian(c,x,y) = select(c == HESSIAN_XX, dxx,
                    select(c == HESSIAN_YY, dyy,
                    select(c == HESSIAN_SS, dss,
                    select(c == HESSIAN_XY, dxy,
                    select(c == HESSIAN_XS, dxs,
                                            dys)))));

    DECL_FUNC(pc_det);
    pc_det(x,y) = hessian(HESSIAN_XX,x,y) * hessian(HESSIAN_YY,x,y) - 2.0f * hessian(HESSIAN_XY,x,y);

    DECL_FUNC(pc_tr);
    pc_tr(x,y)  = hessian(HESSIAN_XX,x,y) + hessian(HESSIAN_YY,x,y);

    DECL_FUNC(invdet);
    invdet(x,y) = 1.0f/(  ( hessian(HESSIAN_XX,x,y) * (hessian(HESSIAN_YY,x,y) * hessian(HESSIAN_SS,x,y) - hessian(HESSIAN_YS,x,y) * hessian(HESSIAN_YS,x,y)) )
                        - ( hessian(HESSIAN_XY,x,y) * (hessian(HESSIAN_XY,x,y) * hessian(HESSIAN_SS,x,y) - hessian(HESSIAN_YS,x,y) * hessian(HESSIAN_XS,x,y)) )
                        + ( hessian(HESSIAN_XS,x,y) * (hessian(HESSIAN_XY,x,y) * hessian(HESSIAN_YS,x,y) - hessian(HESSIAN_YY,x,y) * hessian(HESSIAN_XS,x,y)) ));

    DECL_FUNC(inv);
    inv(c,x,y) = select(c == HESSIAN_XX, (invdet(x,y)) * (hessian(HESSIAN_YY,x,y) * hessian(HESSIAN_SS,x,y) - hessian(HESSIAN_YS,x,y) * hessian(HESSIAN_YS,x,y)),
                select(c == HESSIAN_YY, (invdet(x,y)) * (hessian(HESSIAN_XX,x,y) * hessian(HESSIAN_SS,x,y) - hessian(HESSIAN_XS,x,y) * hessian(HESSIAN_XS,x,y)),
                select(c == HESSIAN_SS, (invdet(x,y)) * (hessian(HESSIAN_XX,x,y) * hessian(HESSIAN_YY,x,y) - hessian(HESSIAN_XY,x,y) * hessian(HESSIAN_XY,x,y)),
                select(c == HESSIAN_XY, (invdet(x,y)) * (hessian(HESSIAN_XS,x,y) * hessian(HESSIAN_YS,x,y) - hessian(HESSIAN_XY,x,y) * hessian(HESSIAN_SS,x,y)),
                select(c == HESSIAN_XS, (invdet(x,y)) * (hessian(HESSIAN_XY,x,y) * hessian(HESSIAN_YS,x,y) - hessian(HESSIAN_XS,x,y) * hessian(HESSIAN_YY,x,y)),
                                        (invdet(x,y)) * (hessian(HESSIAN_XY,x,y) * hessian(HESSIAN_XS,x,y) - hessian(HESSIAN_XX,x,y) * hessian(HESSIAN_YS,x,y)))))));

    DECL_FUNC(interp);
    interp(c,x,y) = select(c == 0, inv(HESSIAN_XX, x,y) * dx(x,y) + inv(HESSIAN_XY, x,y) * dy(x,y) + inv(HESSIAN_XS, x,y) * ds(x,y),
                    select(c == 1, inv(HESSIAN_XY, x,y) * dx(x,y) + inv(HESSIAN_YY, x,y) * dy(x,y) + inv(HESSIAN_YS, x,y) * ds(x,y),
                                  inv(HESSIAN_XS, x,y) * dx(x,y) + inv(HESSIAN_YS, x,y) * dy(x,y) + inv(HESSIAN_SS, x,y) * ds(x,y)));

    DECL_FUNC(interp_contr);
    interp_contr(x,y) = interp(0,x,y) * dx(x,y) + interp(1,x,y) * dy(x,y) + interp(2,x,y) * ds(x,y);

    Expr is_valid = is_extremum(x,y) &&
                    pc_det(x,y) > 0.0f &&
                    (pc_tr(x,y) * pc_tr(x,y) / pc_det(x,y) < ( curv_thr + 1.0f )*( curv_thr + 1.0f ) / curv_thr) &&
                    abs(interp_contr(x,y)) > contr_thr / intervals &&
                    dx(x,y) < 1.0f &&
                    dy(x,y) < 1.0f &&
                    ds(x,y) < 1.0f;

    key(over.x, over.y-1,x,y) = is_valid;

    hessian.parallel(y).vectorize(x, 4);
    pc_det.parallel(y).vectorize(x, 4);
    pc_tr.parallel(y).vectorize(x, 4);
    invdet.parallel(y).vectorize(x, 4);
    inv.parallel(y).vectorize(x, 4);
    interp.parallel(y).vectorize(x, 4);
    interp_contr.parallel(y).vectorize(x, 4);
    gray.parallel(y).vectorize(x, 4);
    
    Expr expr_comb = false;
    expr_comb = select(x % (1 << over.x) == 0 && y % (1 << over.x) == 0, (key(over.x, over.y, x / (1 << over.x), y / (1 << over.x)) || expr_comb), expr_comb);

    Func comb_ext;
    comb_ext(x,y) = 255 * cast<uint8_t>(expr_comb);
    comb_ext.parallel(y).vectorize(x, 4);

    //comb_ext.compileJIT();*/

    /*Func key[octaves][intervals];
    for(int o = 0; o < octaves; o++ )
      for(int i = 1; i <= intervals; i++ ) {

          // 1. Extrema detection
          Expr v = dog_pyr[o][i](x,y);

          RDom r(-1,2,-1,2);

          Expr dmax = max(dog_pyr[o][i+1](x+r.x,y+r.y), max(dog_pyr[o][i  ](x+r.x,y+r.y), dog_pyr[o][i-1](x+r.x,y+r.y)));

          DECL_FUNC(dog_max);
          dog_max(x,y) = -FLT_MAX;
          dog_max(x,y) = max(dog_max(x,y), dmax);

          Expr dmin = min(dog_pyr[o][i+1](x+r.x,y+r.y), min(dog_pyr[o][i  ](x+r.x,y+r.y), dog_pyr[o][i-1](x+r.x,y+r.y)));

          DECL_FUNC(dog_min);
          dog_min(x,y) = FLT_MAX;
          dog_min(x,y) = min(dog_min(x,y), dmin);

          DECL_FUNC(is_extremum);
          is_extremum(x,y) = ( ((abs(v) > prelim_contr_thr) && (v <= 0.0f) && (v == dog_min(x,y))) || (v >  0.0f && v == dog_max(x,y)) );

          // 2. Keypoint localization
          // a. taylor series expansion
          DECL_FUNC(dx);
          dx(x,y) = (dog_pyr[o][i](x+1, y) - dog_pyr[o][i](x-1, y)) / 2.0f;

          DECL_FUNC(dy);
          dy(x,y) = (dog_pyr[o][i](x, y+1) - dog_pyr[o][i](x, y-1)) / 2.0f;

          DECL_FUNC(ds);
          ds(x,y) = (dog_pyr[o][i+1](x, y) - dog_pyr[o][i-1](x, y)) / 2.0f;

          Expr dxx = dog_pyr[o][i  ](x+1, y  ) + dog_pyr[o][i  ](x-1, y  ) - 2.0f * v;
          Expr dyy = dog_pyr[o][i  ](x,   y+1) + dog_pyr[o][i  ](x,   y-1) - 2.0f * v;
          Expr dss = dog_pyr[o][i+1](x,   y  ) + dog_pyr[o][i-1](x,   y  ) - 2.0f * v;
          Expr dxy = ( dog_pyr[o][i  ](x+1, y+1) - dog_pyr[o][i  ](x-1, y+1) - dog_pyr[o][i  ](x+1, y-1) + dog_pyr[o][i  ](x-1, y-1) ) / 4.0f;
          Expr dxs = ( dog_pyr[o][i+1](x+1, y  ) - dog_pyr[o][i+1](x-1, y  ) - dog_pyr[o][i-1](x+1, y  ) + dog_pyr[o][i-1](x-1, y  ) ) / 4.0f;
          Expr dys = ( dog_pyr[o][i+1](x,   y+1) - dog_pyr[o][i+1](x,   y-1) - dog_pyr[o][i-1](x,   y+1) + dog_pyr[o][i-1](x,   y-1) ) / 4.0f;

          // b. Hessian on 2x2 windows
          #define HESSIAN_XX 0
          #define HESSIAN_YY 1
          #define HESSIAN_SS 2
          #define HESSIAN_XY 3
          #define HESSIAN_XS 4
          #define HESSIAN_YS 5

          DECL_FUNC(hessian);
          hessian(c,x,y) = select(c == HESSIAN_XX, dxx,
                          select(c == HESSIAN_YY, dyy,
                          select(c == HESSIAN_SS, dss,
                          select(c == HESSIAN_XY, dxy,
                          select(c == HESSIAN_XS, dxs,
                                                  dys)))));

          DECL_FUNC(pc_det);
          pc_det(x,y) = hessian(HESSIAN_XX,x,y) * hessian(HESSIAN_YY,x,y) - 2.0f * hessian(HESSIAN_XY,x,y);

          DECL_FUNC(pc_tr);
          pc_tr(x,y)  = hessian(HESSIAN_XX,x,y) + hessian(HESSIAN_YY,x,y);

          DECL_FUNC(invdet);
          invdet(x,y) = 1.0f/(  ( hessian(HESSIAN_XX,x,y) * (hessian(HESSIAN_YY,x,y) * hessian(HESSIAN_SS,x,y) - hessian(HESSIAN_YS,x,y) * hessian(HESSIAN_YS,x,y)) )
                              - ( hessian(HESSIAN_XY,x,y) * (hessian(HESSIAN_XY,x,y) * hessian(HESSIAN_SS,x,y) - hessian(HESSIAN_YS,x,y) * hessian(HESSIAN_XS,x,y)) )
                              + ( hessian(HESSIAN_XS,x,y) * (hessian(HESSIAN_XY,x,y) * hessian(HESSIAN_YS,x,y) - hessian(HESSIAN_YY,x,y) * hessian(HESSIAN_XS,x,y)) ));

          DECL_FUNC(inv);
          inv(c,x,y) = select(c == HESSIAN_XX, (invdet(x,y)) * (hessian(HESSIAN_YY,x,y) * hessian(HESSIAN_SS,x,y) - hessian(HESSIAN_YS,x,y) * hessian(HESSIAN_YS,x,y)),
                      select(c == HESSIAN_YY, (invdet(x,y)) * (hessian(HESSIAN_XX,x,y) * hessian(HESSIAN_SS,x,y) - hessian(HESSIAN_XS,x,y) * hessian(HESSIAN_XS,x,y)),
                      select(c == HESSIAN_SS, (invdet(x,y)) * (hessian(HESSIAN_XX,x,y) * hessian(HESSIAN_YY,x,y) - hessian(HESSIAN_XY,x,y) * hessian(HESSIAN_XY,x,y)),
                      select(c == HESSIAN_XY, (invdet(x,y)) * (hessian(HESSIAN_XS,x,y) * hessian(HESSIAN_YS,x,y) - hessian(HESSIAN_XY,x,y) * hessian(HESSIAN_SS,x,y)),
                      select(c == HESSIAN_XS, (invdet(x,y)) * (hessian(HESSIAN_XY,x,y) * hessian(HESSIAN_YS,x,y) - hessian(HESSIAN_XS,x,y) * hessian(HESSIAN_YY,x,y)),
                                              (invdet(x,y)) * (hessian(HESSIAN_XY,x,y) * hessian(HESSIAN_XS,x,y) - hessian(HESSIAN_XX,x,y) * hessian(HESSIAN_YS,x,y)))))));

          DECL_FUNC(interp);
          interp(c,x,y) = select(c == 0, inv(HESSIAN_XX, x,y) * dx(x,y) + inv(HESSIAN_XY, x,y) * dy(x,y) + inv(HESSIAN_XS, x,y) * ds(x,y),
                          select(c == 1, inv(HESSIAN_XY, x,y) * dx(x,y) + inv(HESSIAN_YY, x,y) * dy(x,y) + inv(HESSIAN_YS, x,y) * ds(x,y),
                                        inv(HESSIAN_XS, x,y) * dx(x,y) + inv(HESSIAN_YS, x,y) * dy(x,y) + inv(HESSIAN_SS, x,y) * ds(x,y)));

          DECL_FUNC(interp_contr);
          interp_contr(x,y) = interp(0,x,y) * dx(x,y) + interp(1,x,y) * dy(x,y) + interp(2,x,y) * ds(x,y);

          //Expr is_valid = is_extremum(x,y) && pc_det(x,y) > 0.0f &&
                          //(pc_tr(x,y) * pc_tr(x,y) / pc_det(x,y) < ( curv_thr + 1.0f )*( curv_thr + 1.0f ) / curv_thr) &&
                          //abs(interp_contr(x,y)) > contr_thr / intervals &&
                          //dx(x,y) < 1.0f &&
                          //dy(x,y) < 1.0f &&
                          //ds(x,y) < 1.0f;
          Expr is_valid = is_extremum(x,y) && (pc_det(x,y)) > 0.0f && 
                          ( (pc_tr(x,y) * pc_tr(x,y) / pc_det(x,y)) < ( (curv_thr + 1.0f)*(curv_thr + 1.0f ) / curv_thr) ) && 
                          ( abs(interp_contr(x,y)) > (contr_thr / (float)intervals) ) && 
                          dx(x,y) < 1.0f &&
                          dy(x,y) < 1.0f &&
                          ds(x,y) < 1.0f;

          key[o][i-1](x,y) = is_valid;

          hessian.compute_root().parallel(y).vectorize(x, 4);
          pc_det.compute_root().parallel(y).vectorize(x, 4);
          pc_tr.compute_root().parallel(y).vectorize(x, 4);
          invdet.compute_root().parallel(y).vectorize(x, 4);
          inv.compute_root().parallel(y).vectorize(x, 4);
          interp.compute_root().parallel(y).vectorize(x, 4);
          interp_contr.compute_root().parallel(y).vectorize(x, 4);
        }
          //gray.parallel(y).vectorize(x, 4);

     Expr expr_comb = false;

     for(int o = 0; o < octaves; o++ )
       for(int i = 0; i < intervals; i++ )
         {
           expr_comb = Halide::mux( ( x % (1 << o) == 0 ) && ( y % (1 << o) == 0 ), { Halide::cast<bool>((key[o][i]( (x / (1 << o)) , (y / (1 << o)) )) > 0) || Halide::cast<bool>(expr_comb), Halide::cast<bool>(expr_comb) } );
         }*/

    /*Func comb_ext;
    comb_ext(x,y) = 255 * cast<uint8_t>(expr_comb);
    comb_ext.parallel(y).vectorize(x, 4);
    out = comb_ext;*/
    out(x, y) = Halide::cast<uint8_t>(0);
  }
};

HALIDE_REGISTER_GENERATOR(SIFT, sift);