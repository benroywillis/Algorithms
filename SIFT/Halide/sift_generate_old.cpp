
#include "Halide.h"

using namespace Halide;

#include <iostream>
#include <limits>
#include <memory>
#include <cfloat>
#include <vector>
#include <sys/time.h>

Var x("x"), y("y"), c("c"), k("k"), fi("fi");

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

// Downsample with a 1 3 3 1 filter
Func downsample(Func gray) {
    DECL_FUNC(downx)
    DECL_FUNC(downy)

    downx(x, y) = (gray(2*x-1, y) + 3.0f * (gray(2*x, y) + gray(2*x+1, y)) + gray(2*x+2, y)) / 8.0f;
    downy(x, y) = (downx(x, 2*y-1) + 3.0f * (downx(x, 2*y) + downx(x, 2*y+1)) + downx(x, 2*y+2)) / 8.0f;

    downx.compute_root().parallel(y).vectorize(x, 4);
    downy.compute_root().parallel(y).vectorize(x, 4);

    return downy;
}

Func blur(Func gray, const float sigma) {
  DECL_FUNC(gaussian)

  gaussian(x) = exp(-(x/sigma)*(x/sigma)*0.5f);

  // truncate to 3 sigma and normalize
  int radius = int(3*sigma + 1.0f);
  RDom i(-radius, 2*radius+1);

  DECL_FUNC(normalized)
  normalized(x) = gaussian(x) / sum(gaussian(i)); // Uses an inline reduction

  // Convolve the input using two reductions
  DECL_FUNC(blurx)
  DECL_FUNC(blury)
  blurx(x, y) += gray(x+i, y) * normalized(i);
  blury(x, y) += blurx(x, y+i) * normalized(i);

  blurx.update().parallel(y).vectorize(x, 4);
  blury.update().parallel(y).vectorize(x, 4);

  return blury;
}

class SIFT : public Generator<SIFT> {
public:
  #define SIGMA 1.6f
  Input<Buffer<float>> input{"input", 3};
  const int octaves = 4;
  const int intervals = 5;
  //Input<int> octaves{"octaves", 1};
  //Input<int> intervals{"intervals", 1};
  Input<float> curv_thr{"curv_thr", 1};
  Input<float> contr_thr{"contr_thr", 1};
  Output<Buffer<uint8_t>> out{"out", 2};

  void generate() 
  {
    DECL_FUNC(gray);
    gray(x, y) = 0.299f * input(x, y, 0) + 0.587f * input(x, y, 1) + 0.114f * input(x, y, 2);

    DECL_FUNC(clamped);
    clamped(x, y) = gray(clamp(x, 0, input.width()  - 1), clamp(y, 0, input.height() - 1));

    /* precompute gaussian sigmas */
    /*Buffer<float> sig, sig_prev, sig_total;

    sig[0] = SIGMA;
    float p = pow( 2.0, 1.0 / intervals );
    for(int i = 1; i < intervals + 3; i++ )
    {
      sig_prev = powf( p, i - 1 ) * SIGMA;
      sig_total = sig_prev * p;
      sig[i] = sqrtf( sig_total * sig_total - sig_prev * sig_prev );
    }*/

    Func gauss_pyr[octaves][intervals+3];
    //Expr p = Halide::pow( 2.0f, 1.0f / Halide::cast<float>(intervals) );

    /*Func gauss_pyr("gauss_pyr");
    gauss_pyr(x,y) = clamped(x,y);
    RDom oct(0, octaves, 0, intervals+3);
    gauss_pyr(oct.x, oct.y) = Halide::select( (oct.x == 0) && (oct.y == 0), clamped,
                                              (oct.y == 0), downsample( gauss_pyr(oct.x-1, oct.y) ),
                                              blur( gauss_pyr(oct.x, oct.y-1), Halide::sqrt( Halide::pow( p, oct.y - 1 )*p * Halide::pow( p, oct.y - 1 )*p - Halide::pow( p, oct.y - 1 ) * SIGMA * Halide::pow( p, oct.y - 1 ) * SIGMA ) ) );*/
    float p = (float)pow( 2.0f, 1.0f / (float)intervals);
    for( int o = 0; o < octaves; o++ )
      for( int i = 0; i < intervals + 3; i++ )
      {
        if( o == 0  &&  i == 0 ) {
          gauss_pyr[o][i] = clamped;
        }
        // base of new octave is halved image from end of previous octave
        else if( i == 0 ) {
          gauss_pyr[o][i] = downsample( gauss_pyr[o-1][i] );
        }
        // blur the current octave's last image to create the next one
        else {
          gauss_pyr[o][i] = blur(gauss_pyr[o][i-1], (float)sqrt( (float)pow( p, i - 1 )*p * (float)pow( p, i - 1 )*p - (float)pow( p, i - 1 ) * SIGMA * (float)pow( p, i - 1 ) * SIGMA) );
        }
      }

    /* difference-of-gaussians pyramid */
    Func dog_pyr[octaves][intervals+2];
    /*Func dog_pyr("dog_pyr");

    RDom intv(0, octaves, 0, intervals+2);
    dog_pyr(intv.x, intv.y, x, y) = gauss_pyr(intv.x, intv.y+1) - gauss_pyr(intv.x, intv.y);*/
    
    for(int o = 0; o < octaves; o++ )
      for(int i = 0; i < intervals + 2; i++ ) {
        //int W = input.width()  / (1<<o);
        //int H = input.height() / (1<<o);

        DECL_FUNC(dog_pyr__)
        dog_pyr[o][i] = dog_pyr__;

        dog_pyr[o][i](x,y) = gauss_pyr[o][i+1](x,y) - gauss_pyr[o][i](x,y);
      }
    
    Expr prelim_contr_thr = 0.5f * contr_thr / Halide::cast<float>(intervals);
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

    Func key[octaves][intervals];
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
         }

    Func comb_ext;
    comb_ext(x,y) = 255 * cast<uint8_t>(expr_comb);
    comb_ext.parallel(y).vectorize(x, 4);
    out = comb_ext;
  }
};

HALIDE_REGISTER_GENERATOR(SIFT, sift);