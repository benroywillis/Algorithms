#include "Halide.h"
#include "halide_trace_config.h"
#include "halide_image_io.h"
#include <math.h>
#include <float.h>

namespace {

class HistogramEqualization : public Halide::Generator<HistogramEqualization> {
public:
	// input image is grayscale 640x480
    Input<Buffer<int>> input{"input", 2};
	Input<int> num_bins{"num_bins"};
    Output<Buffer<int>> HistEq{"HistEq", 2};

    void generate() {
		// induction over the input image
		Var x("x"), y("y");
		// function for reading in the pixels in the correct order
		Func in("in");
		in(x, y) = Halide::cast<uint8_t>(clamp( input(x, y), 0, 255 ));
		// histogram
		// this says that hist must go over its entire domain before cdf can occur
        Func hist_rows("hist_rows");
        hist_rows(x, y) = 0;
        RDom rx(0, input.width());
        Expr bin = cast<int>(in(rx, y));
        hist_rows(bin, y) += 1;

        Func hist("hist");
        hist(x) = 0;
        RDom ry(0, input.height());
        hist(x) += hist_rows(x, ry);
		// compute CDF
        Func cdf("cdf");
        cdf(x) = hist(0);
        RDom b(1, 255);
        cdf(b.x) = cdf(b.x - 1) + hist(b.x);

        Func cdf_bin("cdf_bin");
        cdf_bin(x, y) = in(x, y);

        Func eq("equalize");
        eq(x, y) = clamp(cdf(cdf_bin(x, y)) * (255.0f / (input.height() * input.width())), 0, 255);

		// Map input pixels to their transformed saturation level
		//HistEq(y, x) = cast<int>(lut(in( x,y )));
		HistEq(x, y) = 0;
		HistEq(x, y) = cast<int>(eq(x, y));

		// schedule
		const int vec = natural_vector_size<float>();
        RVar rxo, rxi;
        in.clone_in(hist_rows).compute_at(hist_rows.in(), y).vectorize(x, vec);
        hist_rows.in().compute_root().vectorize(x, vec).parallel(y, 4);
        hist_rows.compute_at(hist_rows.in(), y).vectorize(x, vec).update().reorder(y, rx).unroll(y);
        hist.compute_root().vectorize(x, vec).update().reorder(x, ry).vectorize(x, vec).unroll(x, 4).parallel(x).reorder(ry, x);
        cdf.compute_root();
        HistEq.reorder(x, y).parallel(y, 8).vectorize(x, vec * 2);
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(HistogramEqualization, HistEq)
