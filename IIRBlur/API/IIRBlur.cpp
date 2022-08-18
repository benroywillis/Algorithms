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
	Mat K(2, 1, CV_32FC1, &iirKernel);

	// IIR filter takes the previous row and filters it with the current row, along the columns
	// Algorithm:
	// 1. filter from 0 to num_rows-1
	// 2. transpose
	// 3. do step 1 but from num_rows-1 to 0
	// 4. transpose again
	// source, dest, output image depth (-1 means same as the input image), filter, filtered point position, pixel offset, border
	__TIMINGLIB_benchmark( [&]() { 
  	filter2D( src, dst, -1, K, Point(0,1), 0, BORDER_REPLICATE ); // border replicate -> aaa | abc | ccc
  	transpose( dst, dst );
  	filter2D( dst, dst, -1, K, Point(0,1), 0, BORDER_REPLICATE );
  	transpose( dst, dst );
	} );

	imwrite(argv[3], dst);
	
 	return 0;
}
