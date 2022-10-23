#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include "TimingLib.h"
#include <string>
#include <time.h>
#include <iostream>

using namespace cv;
using namespace std;

int main( int argc, char** argv )
{
	if( argc != 5 )
	{
		cout << "Please specify (float)alpha, input image path, output image path, and number of threads" << endl;
		return EXIT_FAILURE;
	}
	setNumThreads(atoi(argv[4]));
	float alpha = std::strtof(argv[1], NULL);
	Mat src = imread( argv[2] );
	Mat dst;
	float iirKernel[2][1] = { {1-alpha}, {alpha} };
	Mat K(1, 2, CV_32FC1, &iirKernel);
	// anchor for going down columns
	auto anchor_down = Point(1, 0);
	// anchor for going up columns
	auto anchor_up = Point(0, 0);
	//float iirKernel[3][3] = { {0.11f, 0.11f, 0.11f}, {0.11f, 0.11f, 0.11f}, {0.11f, 0.11f, 0.11f} };
	//Mat K(3, 3, CV_32FC1, &iirKernel);

	// IIR filter takes the previous row and filters it with the current row, along the columns
	// Algorithm:
	// 1. filter from 0 to num_rows-1
	// 2. do step 1 but from num_rows-1 to 0
	// 3. transpose
	// 4. do step 1
	// 5. do step 4 but from num_rows-1 to 0
	// 6. transpose again
	// source, dest, output image depth (-1 means same as the input image), filter, filtered point position, pixel offset, border
	__TIMINGLIB_benchmark( [&]() { 
		Mat blur0;
  		filter2D( src, blur0, -1, K, anchor_down, 0, BORDER_REPLICATE ); // border replicate -> aaa | abc | ccc
		Mat blur1;
  		filter2D( blur0, blur1, -1, K, anchor_up, 0, BORDER_REPLICATE ); // border replicate -> aaa | abc | ccc
  		blur1 = blur1.t();
		Mat blur2;
  		filter2D( blur1, blur2, -1, K, anchor_down, 0, BORDER_REPLICATE );
		Mat blur3;
  		filter2D( blur2, blur3, -1, K, anchor_up, 0, BORDER_REPLICATE );
  		dst = blur3.t();
	} );

	imwrite(argv[3], dst);
	
 	return 0;
}
