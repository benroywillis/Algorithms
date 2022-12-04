
#include "harris.h"

#include "TimingLib.h"
#include "BilateralFilter.h"
#include <iostream>

void detect_corners(float* x, int nx, int ny,
                    float k=0.060000,
                    float sigma_d=1.000000,
                    float sigma_i=2.500000,
                    float threshold=130,
                    int gaussian=1,
                    int gradient=0,
                    int strategy=0,
                    int Nselect=1,
                    int measure=0,
                    int Nscales=1,
                    int precision=1,
                    int cells=10,
                    int verbose=1) {
  std::vector<harris_corner> corners;
  float *I=new float[nx*ny];
  for(int i = 0; i < nx*ny; i++) 
	I[i] = (float)x[i];

  harris_scale(
    I, corners, Nscales, gaussian, gradient, measure, k,
    sigma_d, sigma_i, threshold, strategy, cells, Nselect,
    precision, nx, ny, verbose
  );

  unsigned int nr_corners = corners.size();
  std::vector<float> loc_x;
  std::vector<float> loc_y;
  std::vector<float> loc_strength;
  for (unsigned int i = 0; i < nr_corners; i++){
    loc_x.push_back(corners[i].x);
    loc_y.push_back(corners[i].y);
    loc_strength.push_back(corners[i].R);
  }
}


int main(int argc, char** argv)
{
	if( argc != 4 )
	{
		std::cout << "Please input <input image> <output image> <threads>" << std::endl;
		return EXIT_FAILURE;
	}
	std::string inpath = std::string(argv[1]);
	struct Pixel* input  = readImage(argv[2]);
    struct Pixel* output = (struct Pixel*)calloc(image_width*image_height, sizeof(struct Pixel));
    printf("Image size: %d x %d\n", image_height, image_width);

	// convert to grayscale floating point
	float* gray = (float*)calloc(image_width*image_height, sizeof(float));
	for( unsigned int y = 0; y < image_height; y++ )
	{
		for( unsigned int x = 0; x < image_width; x++ )
		{
			gray[y*image_width+x] = 0.2989f*input[y*image_width+x].r + 
									0.5870f*input[y*image_width+x].g + 
									0.1140f*input[y*image_width+x].b;
		}
	}

	__TIMINGLIB_benchmark( [&] { detect_corners(gray, image_width, image_height); } );

	return 0;
}
