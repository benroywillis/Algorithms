// from https://github.com/kuan-wang/The_Bilateral_Solver

#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/ximgproc/edge_filter.hpp"
#include "TimingLib.h"

using namespace std;
using namespace cv;

int main(int argc, char** argv)
{
	setNumThreads(0);
	double sigma_s = std::stof(argv[1]);
	double sigma_r = std::stof(argv[2]);
	Mat src = imread( argv[3], IMREAD_COLOR );
	Mat gra; cvtColor(src, gra, COLOR_BGR2GRAY);
	Mat dst;

	// actual image (known to the API as "guide" or "reference"), sigma_spatial, sigma_luma, sigma_chroma, lambda, num_iter, max_tol
  	auto fbs = ximgproc::createFastBilateralSolverFilter( src, sigma_s, sigma_r, sigma_r );
	__TIMINGLIB_start_time();
	fbs->filter(src, gra, dst);
	__TIMINGLIB_end_time();
	imwrite(argv[4], dst);
	
	return 0;
}
