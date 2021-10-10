#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include <string>
#include <time.h>

using namespace cv;

int main( int argc, char** argv )
{
	setNumThreads(0);
	double sigma_s = std::stof(argv[1]);
	double sigma_r = std::stof(argv[2]);
	Mat src = imread( argv[3] );
	Mat dst;

	struct timespec start;
	struct timespec end;
	while(clock_gettime(CLOCK_MONOTONIC, &start)) {}
  	bilateralFilter( src, dst, 5, sigma_r, sigma_s );
	while(clock_gettime(CLOCK_MONOTONIC, &end)) {}
	double secdiff = (double)(end.tv_sec - start.tv_sec);
	double nsecdiff = (double)(end.tv_nsec - start.tv_nsec) * pow(10.0, -9.0);
	double totalTime = secdiff + nsecdiff;
	printf("Bilateral filter runtime: %fs\n", totalTime);

	imwrite(argv[4], dst);
	
 	return 0;
}
