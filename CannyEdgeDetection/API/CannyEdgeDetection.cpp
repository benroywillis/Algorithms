#include <opencv2/imgproc.hpp>
#include "opencv2/core/utility.hpp"
#include "opencv2/imgcodecs.hpp"
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
    setNumThreads(0);
	if( argc != 5 )
	{
		cout << "Please specify hysteresis threshold 1, 2, then input filename and output filename!" << endl;
	}
	double thresh1 = stod(argv[1]);
	double thresh2 = stod(argv[2]);
    auto image = imread(samples::findFile(string(argv[3])), IMREAD_COLOR);
    if(image.empty())
    {
        printf("Cannot read image file: %s\n", argv[3]);
        return -1;
    }
	Mat edges;
	cv::Canny(image, edges, thresh1, thresh2);
	imwrite(string(argv[4]), edges);
	cout << "Success!" << endl;
	return 0;
}
