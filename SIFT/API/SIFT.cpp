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
	if( argc != 8 )
	{
		cout << "Please specify octaves, thresholds, curve threshold, contrast threshold, input image path, output image path, and number of threads" << endl;
		return EXIT_FAILURE;
	}
	setNumThreads(atoi(argv[7]));
	int octaves = atoi(argv[1]);
	int thresholds = atoi(argv[2]);
	float curve_threshold= strtof(argv[3], NULL);
	float contrast_threshold = strtof(argv[4], NULL);
	Mat src = imread( argv[5] );
	Mat gray;
	cv::cvtColor(src, gray, COLOR_BGR2GRAY);
	Mat dst;

	__TIMINGLIB_benchmark( [&]() { 
		auto sift = SIFT::create( 0, octaves, contrast_threshold, curve_threshold);
		auto keys = vector<KeyPoint>();
		sift->detect(gray, keys);
		drawKeypoints(gray, keys, dst);  
	} );

	imwrite(argv[6], dst);
	
 	return 0;
}
