#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <iostream>
#include "TimingLib.h"
#include "BilateralFilter.h"

using namespace std;

#define PRECISION 	float

// gaussian kernel parameters
#define K	5
#define L	5

// discrete gaussian kernel approximation with mu = 0 sigma = 1
const PRECISION filter[K][L] = {
                                 {1.0f / 273.0f,  4.0f / 273.0f,  7.0f / 273.0f,  4.0f / 273.0f, 1.0f / 273.0f},
                                 {4.0f / 273.0f, 16.0f / 273.0f, 26.0f / 273.0f, 16.0f / 273.0f, 4.0f / 273.0f},
                                 {7.0f / 273.0f, 26.0f / 273.0f, 41.0f / 273.0f, 26.0f / 273.0f, 7.0f / 273.0f},
                                 {4.0f / 273.0f, 16.0f / 273.0f, 26.0f / 273.0f, 16.0f / 273.0f, 4.0f / 273.0f},
                                 {1.0f / 273.0f,  4.0f / 273.0f,  7.0f / 273.0f,  4.0f / 273.0f, 1.0f / 273.0f}
                               };

struct Corner {
	int y; // row
	int x; // col
	PRECISION R;
	Corner(int xx, int yy, PRECISION RR) : y(yy), x(xx), R(RR) {}
	Corner() : y(0), x(0), R(0) {}
};

void gaussian(PRECISION* in, PRECISION* blur, unsigned height, unsigned width)
{
	#pragma omp parallel for
	for( int y = 0; y < height; y++ )
	{
		for( int x = 0; x < width; x++ )
		{
			for( int k = -K/2; k < K/2; k++ )
			{
				int row = y+k > height ? y-k : abs(y-k);
				for( int l = -L/2; l < L/2; l++ )
				{
					int col = x+l > width ? x-l : abs(x-l);
					blur[y*width + x] += in[row*width + col]*filter[k+K/2][l+L/2];
				}
			}
		}
	}
}

void sobel(PRECISION* blur, PRECISION* Ix, PRECISION* Iy, unsigned height, unsigned width)
{
	#pragma omp parallel for
	for( unsigned y = 0; y < height; y++ )
	{
		for( unsigned x = 0; x < width; x++ )
		{
			if( y == 0 || y == height-1 || x == 0 || x == width-1 )
			{
				Ix[y*width + x] = 0;
				Iy[y*width + x] = 0;
			}
			else
			{
				Ix[y*width + x] = blur[  y   *width + x-1 ] + (PRECISION)(-1)*blur[  y   *width + x+1 ];
				Iy[y*width + x] = blur[ (y-1)*width + x   ] + (PRECISION)(-1)*blur[ (y+1)*width + x   ];
			}
		}
	}
}

void corr(PRECISION* Ix, PRECISION* Iy, PRECISION* Ixx, PRECISION* Iyy, PRECISION* Ixy, unsigned height, unsigned width)
{
	for( unsigned y = 0; y < image_height; y++ )
	{
		for( unsigned x = 0; x < image_width; x++ )
		{
			Ixx[y*width + x] = Ix[y*width + x]*Ix[y*width + x];
			Iyy[y*width + x] = Iy[y*width + x]*Iy[y*width + x];
			Ixy[y*width + x] = Ix[y*width + x]*Iy[y*width + x];
		}
	}
}

void cornerResponse(PRECISION* Ixx, PRECISION* Iyy, PRECISION* Ixy, vector<struct Corner>& corners, unsigned height, unsigned width)
{
	// parameters for corner response and non-maximum suppression
	PRECISION k = (PRECISION)0.04; // coefficient to regulate corner response function
	PRECISION Th = (PRECISION)1; // threshold to determine corner responses that are too low
	int radius  = 5; // corner window radius

	// array of corner responses
	PRECISION* R = (PRECISION*)calloc(height*width, sizeof(PRECISION));
	for( unsigned i = 0; i < height*width; i++ )
	{
		PRECISION xx = Ixx[i];
		PRECISION yy = Iyy[i];
		PRECISION xy = Ixy[i];
		R[i] = (Ixx[i]*Ixy[i] - Iyy[i]*Iyy[i]) - k*(Ixx[i]+Ixy[i])*(Ixx[i]+Ixy[i]); // det(Ixx) - k*trace*trace
	}

    int* skip = (int*)calloc(height*width, sizeof(int));
  
    // skip values under the threshold
    #ifdef _OPENMP
    #pragma omp parallel for
    #endif  
    for( unsigned i=0; i < height*width; i++ )
	{
      	if(R[i]<Th) skip[i]=1;
      	else skip[i]=0;
	}

    // use an array for each row to allow parallel processing
    vector<vector<struct Corner> > corners_row(height-2*radius);
 
    #ifdef _OPENMP
    #pragma omp parallel for
    #endif
    for(int i = radius; i < height-radius; i++)
    {
    	int j=radius;
    	// avoid the downhill at the beginning
    	while( (j < width-radius) && (skip[i*width+j] || R[i*width+j-1] >= R[i*width+j]) )
		{
      		j++;
		}
      
    	while( j < width-radius )
    	{
      		// find the next peak 
      		while( (j < width-radius) && (skip[i*width + j] || R[i*width + j+1] >= R[i*width + j]) )
			{
        		j++;
			}
      
      		if( j < width-radius )
      		{
        		int p1 = j+2;
        		// find a bigger value on the right
        		while( (p1 <= j+radius) && (R[i*width + p1] < R[i*width + j]) ) 
        		{
          			skip[i*width + p1] = 1;
          			p1++;
        		}
        		// if not found
        		if( p1 > j+radius )
        		{  
          			int p2 = j-1;
          			// find a bigger value on the left
          			while( (p2 >= j-radius) && (R[i*width + p2] <= R[i*width + j]) )
					{
            			p2--;
					}
          			// if not found, check the 2D region
          			if( p2 < j-radius )
          			{
            			int      k = i + radius; 
            			bool found = false;
            			// first check the bottom region (backwards)
            			while( !found && (k > i) )
            			{
              				int l = j + radius;
              				while(!found && l>=j-radius)
              				{
                				if(R[k*width+l]>R[i*width+j])
								{
                  					found=true;
								}
                				else 
								{
									skip[k*width+l]=1;
								}
                				l--;
              				}
              				k--;
            			}
            
            			k= i - radius; 

            			// then check the top region (forwards)
            			while(!found && k<i)
            			{
              				int l = j - radius;
              				while(!found && l<=j+radius)
              				{
                				if(R[k*width+l]>=R[i*width+j])
                  					found=true;
                
                				l++;
              				}
              				k++;
            			}
            
            			if(!found)
						{ 
							// a new local maximum detected 
							corners_row[i-radius].push_back({(int)j, (int)i, R[i*width+j]});
						}
          			}
        		}
        		j=p1;
      		}
    	}
  	}
  
  	// copy row corners to the output list
  	for(int i=0; i< height - 2*radius; i++)
   		corners.insert(corners.end(), corners_row[i].begin(), corners_row[i].end());

	free(R);
	free(skip);

}

int main(int argc, char** argv)
{
	if( argc != 4 )
	{
		std::cout << "Please input <input image> <output image> <threads>" << std::endl;
		return EXIT_FAILURE;
	}
	struct Pixel* input  = readImage(argv[1]);
    struct Pixel* output = (struct Pixel*)calloc(image_width*image_height, sizeof(struct Pixel));
    printf("Image size: %d x %d\n", image_height, image_width);

	// convert to grayscale PRECISIONing point
	PRECISION* gray = (PRECISION*)calloc(image_width*image_height, sizeof(PRECISION));
	for( unsigned int y = 0; y < image_height; y++ )
	{
		for( unsigned int x = 0; x < image_width; x++ )
		{
			gray[y*image_width+x] = (PRECISION)(0.299)*(PRECISION)input[y*image_width+x].r + 
									(PRECISION)(0.587)*input[y*image_width+x].g + 
									(PRECISION)(0.114)*input[y*image_width+x].b;
		}
	}

	// blurred image
	PRECISION* blurred = (PRECISION*)calloc(image_width*image_height, sizeof(PRECISION));
	// sobel products
	PRECISION* Ix = (PRECISION*)calloc(image_width*image_height, sizeof(PRECISION));
	PRECISION* Iy = (PRECISION*)calloc(image_width*image_height, sizeof(PRECISION));
	// autocorrelation
	PRECISION* Ixx = (PRECISION*)calloc(image_width*image_height, sizeof(PRECISION));
	PRECISION* Iyy = (PRECISION*)calloc(image_width*image_height, sizeof(PRECISION));
	PRECISION* Ixy = (PRECISION*)calloc(image_width*image_height, sizeof(PRECISION));
	// detected corners
	vector<struct Corner> corners;
	// Harris algorithm
	__TIMINGLIB_benchmark( [&] { 
		gaussian(gray, blurred, image_height, image_width);
		sobel(blurred, Ix, Iy, image_height, image_width);
		corr(Ix, Iy, Ixx, Iyy, Ixy, image_height, image_width);
		cornerResponse(Ixx, Iyy, Ixy, corners, image_height, image_width);
	} );

	// write output image
	/*for( unsigned y = 0; y < image_height; y++ )
	{
		for( unsigned x = 0; x < image_width; x++ )
		{
			output[y*image_width + x].r = input[y*image_width + x].r;
			output[y*image_width + x].g = input[y*image_width + x].g;
			output[y*image_width + x].b = input[y*image_width + x].b;
		}
	}
	cout << "Detected " << corners.size() << " corners." << endl;
	// draw corners
	for( const auto& corner : corners )
	{
		// draw a 3x3 box around each corner
		int row = corner.y;
		int col = corner.x;
		output[(row-1)*image_width + col-1].r = 255;
		output[(row-1)*image_width + col-1].g = 255;
		output[(row-1)*image_width + col-1].b = 255;
		output[(row-1)*image_width + col  ].r = 255;
		output[(row-1)*image_width + col  ].g = 255;
		output[(row-1)*image_width + col  ].b = 255;
		output[(row-1)*image_width + col+1].r = 255;
		output[(row-1)*image_width + col+1].g = 255;
		output[(row-1)*image_width + col+1].b = 255;
		output[row*image_width + col-1].r = 255;
		output[row*image_width + col-1].g = 255;
		output[row*image_width + col-1].b = 255;
		output[row*image_width + col+1].r = 255;
		output[row*image_width + col+1].g = 255;
		output[row*image_width + col+1].b = 255;
		output[(row+1)*image_width + col-1].r = 255;
		output[(row+1)*image_width + col-1].g = 255;
		output[(row+1)*image_width + col-1].b = 255;
		output[(row+1)*image_width + col  ].r = 255;
		output[(row+1)*image_width + col  ].g = 255;
		output[(row+1)*image_width + col  ].b = 255;
		output[(row+1)*image_width + col+1].r = 255;
		output[(row+1)*image_width + col+1].g = 255;
		output[(row+1)*image_width + col+1].b = 255;
	}

	writeImage(output, argv[2]);*/

	free(input);
	free(output);
	free(gray);
	free(blurred);
	free(Ix);
	free(Iy);
	free(Ixx);
	free(Iyy);
	free(Ixy);

	return 0;
}
