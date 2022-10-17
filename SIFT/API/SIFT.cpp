#include "opencv2/features2d.hpp"
#include "opencv2/opencv.hpp"
#include "TimingLib.h"
#include <string>
#include <time.h>
#include <iostream>

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
	Mat gray;
	cv::cvtColor(src, gray, COLOR_BGR2GRAY);
	Mat dst;

	__TIMINGLIB_benchmark( [&]() { 
		auto sift = SIFT::create();
		auto keys = vector<KeyPoint>();
		sift->detect(gray, keys);
		drawKeypoints(gray, keys, dst);  
	} );

	imwrite(argv[2], dst);
	
 	return 0;
}
