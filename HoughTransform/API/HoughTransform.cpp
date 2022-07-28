
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	setNumThreads(0);
	if( argc != 5 )
	{
		cout << "Please input dp, mindist, input and output image";
	}
	double dp = stod(argv[1]);
	double minDist = stod(argv[2]);
    // Loads an image
    Mat src = imread( samples::findFile( string(argv[3]) ), IMREAD_COLOR );
    // Check if image is loaded fine
    if(src.empty()){
        printf(" Error opening image\n");
        printf(" Program Arguments: [image_name -- default %s] \n", argv[3]);
        return EXIT_FAILURE;
    }
    Mat gray;
    cvtColor(src, gray, COLOR_BGR2GRAY);
    medianBlur(gray, gray, 5);
    vector<Vec3f> circles;
	// 100 codes for the highest threshold of the two from canny
	// 30 is an accumulator threshold, the smaller it is the more false circles may be detected
	// 1 is the min radius for a circle
	// 30 is the max radius for a circle
    HoughCircles(gray, circles, HOUGH_GRADIENT, dp, minDist, 100, 30, 1, 30 );
	// this section is for drawing the circles
    for( size_t i = 0; i < circles.size(); i++ )
    {
        Vec3i c = circles[i];
        Point center = Point(c[0], c[1]);
        // circle center
        circle( src, center, 1, Scalar(0,100,100), 3, LINE_AA);
        // circle outline
        int radius = c[2];
        circle( src, center, radius, Scalar(255,0,255), 3, LINE_AA);
    }
    imwrite(string(argv[3]), src);
	printf("Success!\n");
    return EXIT_SUCCESS;
}
