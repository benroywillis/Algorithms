#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include "TimingLib.h"
#include <string>
#include <time.h>
#include <iostream>

using namespace cv;
using namespace std;

const int thresh = 200;
const int max_thresh = 255;

int main( int argc, char** argv )
{
	if( argc != 4 )
	{
		cout << "Please specify input image path, output image path, and number of threads" << endl;
		return EXIT_FAILURE;
	}
	setNumThreads(atoi(argv[3]));
	Mat src = imread( argv[1] );
    Mat dst = Mat::zeros( src.size(), CV_32FC1 );
	Mat src_gray;

    cvtColor( src, src_gray, COLOR_BGR2GRAY );

	__TIMINGLIB_benchmark( [&]() { 
    	int blockSize = 2;
    	int apertureSize = 3;
    	double k = 0.04;
    	cornerHarris( src_gray, dst, blockSize, apertureSize, k );
    	Mat dst_norm, dst_norm_scaled;
    	normalize( dst, dst_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat() );
    	convertScaleAbs( dst_norm, dst_norm_scaled );
    	for( int i = 0; i < dst_norm.rows ; i++ )
    	{
        	for( int j = 0; j < dst_norm.cols; j++ )
        	{
            	if( (int) dst_norm.at<float>(i,j) > thresh )
            	{
                	circle( dst_norm_scaled, Point(j,i), 5,  Scalar(0), 2, 8, 0 );
            	}
        	}
    	}
	} );

	imwrite(argv[2], dst);
	
 	return 0;
}
