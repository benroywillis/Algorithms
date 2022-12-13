#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include "TimingLib.h"
#include <string>
#include <time.h>
#include <iostream>

#ifndef PRECISION
#define PRECISION float
#endif

using namespace cv;
using namespace std;

int main( int argc, char** argv )
{
	if( argc != 4 )
	{
		cout << "Please specify input image path, output image path, and number of threads" << endl;
		return EXIT_FAILURE;
	}
	setNumThreads(atoi(argv[3]));
	Mat src = imread( argv[1] );
	Mat dst;
	// 2D gaussian filter approximation with mu = 0 and sigma = 1.0
	PRECISION gaussianKernel[5][5] = {
                                 {1.0f / 273.0f,  4.0f / 273.0f,  7.0f / 273.0f,  4.0f / 273.0f, 1.0f / 273.0f},
                                 {4.0f / 273.0f, 16.0f / 273.0f, 26.0f / 273.0f, 16.0f / 273.0f, 4.0f / 273.0f},
                                 {7.0f / 273.0f, 26.0f / 273.0f, 41.0f / 273.0f, 26.0f / 273.0f, 7.0f / 273.0f},
                                 {4.0f / 273.0f, 16.0f / 273.0f, 26.0f / 273.0f, 16.0f / 273.0f, 4.0f / 273.0f},
                                 {1.0f / 273.0f,  4.0f / 273.0f,  7.0f / 273.0f,  4.0f / 273.0f, 1.0f / 273.0f}
                               };

	Mat K(5, 5, CV_32FC1, &gaussianKernel);
	// anchor is in the middle of the kernel
	auto filter_anchor = Point(2, 2);

	// source, dest, output image depth (-1 means same as the input image), filter, filtered point position, pixel offset, border
	__TIMINGLIB_benchmark( [&]() { 
		Mat blur0;
  		filter2D( src, blur0, -1, K, filter_anchor, 0, BORDER_REPLICATE ); // border replicate -> aaa | abc | ccc
		Mat blur1;
  		filter2D( blur0, blur1, -1, K, filter_anchor, 0, BORDER_REPLICATE ); // border replicate -> aaa | abc | ccc
		Mat blur2;
  		filter2D( blur1, blur2, -1, K, filter_anchor, 0, BORDER_REPLICATE );
		Mat blur3;
  		filter2D( blur2, blur3, -1, K, filter_anchor, 0, BORDER_REPLICATE );
		Mat blur4;
  		filter2D( blur3, blur4, -1, K, filter_anchor, 0, BORDER_REPLICATE );
  		dst = blur4;
	} );

	imwrite(argv[2], dst);
	
 	return 0;
}
