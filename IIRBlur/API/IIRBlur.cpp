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
	auto anchor = Point(1, 0);
	//float iirKernel[3][3] = { {0.11f, 0.11f, 0.11f}, {0.11f, 0.11f, 0.11f}, {0.11f, 0.11f, 0.11f} };
	//Mat K(3, 3, CV_32FC1, &iirKernel);

	// IIR filter takes the previous row and filters it with the current row, along the columns
	// Algorithm:
	// 1. filter from 0 to num_rows-1
	// 2. transpose
	// 3. do step 1 but from num_rows-1 to 0
	// 4. transpose again
	// source, dest, output image depth (-1 means same as the input image), filter, filtered point position, pixel offset, border
	__TIMINGLIB_benchmark( [&]() { 
		Mat blur0;
  		filter2D( src, blur0, -1, K, anchor, 0, BORDER_REPLICATE ); // border replicate -> aaa | abc | ccc
  		blur0 = blur0.t();
		Mat blur1;
  		filter2D( blur0, blur1, -1, K, anchor, 0, BORDER_REPLICATE );
  		dst = blur1.t();
	} );

	imwrite(argv[3], dst);
	
 	return 0;
}
