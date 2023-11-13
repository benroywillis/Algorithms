#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
//#include "inner_product.h"
//#ifndef NO_AUTO_SCHEDULE
//#include "inner_product_autoschedule_true_generated.h"
#include "inner_product_autoschedule_false_generated.h"
//#endif

#include "HalideBuffer.h"
#include "halide_benchmark.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

#define SIZE 	512

int main(int argc, char **argv)
{	
	if( argc != 3 )
	{
		std::cout << "Please provide input directory and output file name" << std::endl;
		return EXIT_FAILURE;
	}
    Halide::Runtime::Buffer<float> vec_a(nullptr, SIZE);
    Halide::Runtime::Buffer<float> vec_b(nullptr, SIZE);
	vec_a.allocate();
	vec_b.allocate();

    // Manually-tuned version
    int timing_iterations = 15;
    double time = __TIMINGLIB_benchmark( [&] { inner_product_autoschedule_false_generated(vec_a, vec_b); } );
    return 0;
}


