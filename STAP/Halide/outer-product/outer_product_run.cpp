#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
//#include "outer_product.h"
//#ifndef NO_AUTO_SCHEDULE
//#include "outer_product_autoschedule_true_generated.h"
#include "outer_product_autoschedule_false_generated.h"
//#endif

#include "HalideBuffer.h"
#include "halide_benchmark.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

// PERFECT related file reading and parameter setting

#include "stap_params.h"
#include "stap_utils.h"

#if INPUT_SIZE == INPUT_SIZE_SMALL
    static const char *input_filename = "small_input.bin";
    static const char *kernel1_outer_filename = "small_kernel1_output.bin";
#elif INPUT_SIZE == INPUT_SIZE_MEDIUM
    static const char *input_filename = "medium_input.bin";
    static const char *kernel1_outer_filename = "medium_kernel1_output.bin";
#elif INPUT_SIZE == INPUT_SIZE_LARGE
    static const char *input_filename = "large_input.bin";
    static const char *kernel1_outer_filename = "large_kernel1_output.bin";
#else
    #error "Unhandled value for INPUT_SIZE"
#endif

int main(int argc, char **argv)
{	
	if( argc != 3 )
	{
		std::cout << "Please provide input directory and outer file name" << std::endl;
		return EXIT_FAILURE;
	}

    char *input_directory = argv[1];

    complex (*datacube)[N_DOP][N_RANGE] = NULL;
    complex (*covariances)[N_BLOCKS][N_CHAN*TDOF][N_CHAN*TDOF] = NULL;

#ifdef ENABLE_CORRECTNESS_CHECKING
    complex (*gold_covariances)[N_BLOCKS][N_CHAN*TDOF][N_CHAN*TDOF] = NULL;
#endif

    const size_t num_datacube_elements = N_CHAN * N_DOP * N_RANGE;
    const size_t num_covariance_elements = (TDOF*N_CHAN) * (TDOF*N_CHAN) *
        N_DOP * N_BLOCKS;
    

    datacube = (complex (*)[N_DOP][N_RANGE]) malloc(sizeof(complex) * num_datacube_elements);
    covariances = (complex (*)[N_BLOCKS][N_CHAN*TDOF][N_CHAN*TDOF])malloc(sizeof(complex) * num_covariance_elements);

#ifdef ENABLE_CORRECTNESS_CHECKING
    gold_covariances = (complex (*)[N_BLOCKS][N_CHAN*TDOF][N_CHAN*TDOF])malloc(sizeof(complex) * num_covariance_elements);
#endif

    printf("Loading input files from %s...\n", input_directory);

    read_complex_data_file(
        (complex *) datacube,
        input_filename,
        input_directory,
        num_datacube_elements);

#ifdef ENABLE_CORRECTNESS_CHECKING
    read_complex_data_file(
        (complex *) gold_covariances,
        kernel1_outer_filename,
        input_directory,
        num_covariance_elements);
#endif

    memset(covariances, 0, sizeof(complex) * num_covariance_elements);

 	halide_dimension_t datacube_dims[] = {{0, N_CHAN, 1}, {0, N_DOP, 1} , {0, N_RANGE, 1}, {0, 2, 1}};
    Buffer<double> Buffer_datacube( (double*)datacube, 4, datacube_dims);
 	halide_dimension_t covariances_dims[] = {{0, N_DOP, 1}, {0, N_BLOCKS, 1} , {0, TDOF*N_CHAN, 1}, {0, TDOF*N_CHAN, 1}, {0, 2, 1}};
    Buffer<double> Buffer_covariances( (double*)covariances, 5, covariances_dims);

    // Manually-tuned version
    int timing_iterations = 15;
    double min_t_manual = benchmark(timing_iterations, 10, [&]() {
        outer_product_autoschedule_false_generated(Buffer_datacube, Buffer_covariances);
        Buffer_covariances.device_sync();
    });
    printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);

    // Auto-scheduled version
	// Ben [9/06/20] for some reason the autoscheduled version has an improper access to the outer image and fails
    /*double min_t_auto = benchmark(timing_iterations, 10, [&]() {
	outer_product_autoschedule_true_generated(input, r_sigma, outer);
        outer.device_sync();
    });
    printf("Auto-scheduled time: %gms\n", min_t_auto * 1e3);*/

    convert_and_save_image(Buffer_covariances, argv[2]);

#ifdef ENABLE_CORRECTNESS_CHECKING
    {
        double snr;
        printf("\nComputing correctness metrics...\n");
        snr = calculate_snr(
            (complex *) gold_covariances,
            (complex *) covariances,
            num_covariance_elements);
        printf("\tSNR after STAP kernel 1 : %.2f dB\n", snr);
    }
    FREE_AND_NULL(gold_covariances);
#endif
    FREE_AND_NULL(datacube);
    FREE_AND_NULL(covariances);
    return 0;
}


