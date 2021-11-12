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
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

// PERFECT related file reading and parameter setting

#include "stap_params.h"
#include "stap_utils.h"
#include "stap_apply_weighting.h"

#if INPUT_SIZE == INPUT_SIZE_SMALL
    static const char *input_filename = "small_input.bin";
    static const char *kernel2_output_filename = "small_kernel2_output.bin";
    static const char *kernel3_output_filename = "small_kernel3_output.bin";
    static const char *steering_vector_filename = "small_steering_vectors.bin";
#elif INPUT_SIZE == INPUT_SIZE_MEDIUM
    static const char *input_filename = "medium_input.bin";
    static const char *kernel2_output_filename = "medium_kernel2_output.bin";
    static const char *kernel3_output_filename = "medium_kernel3_output.bin";
    static const char *steering_vector_filename = "medium_steering_vectors.bin";
#elif INPUT_SIZE == INPUT_SIZE_LARGE
    static const char *input_filename = "large_input.bin";
    static const char *kernel2_output_filename = "large_kernel2_output.bin";
    static const char *kernel3_output_filename = "large_kernel3_output.bin";
    static const char *steering_vector_filename = "large_steering_vectors.bin";
#else
    #error "Unhandled value for INPUT_SIZE"
#endif

int main(int argc, char **argv)
{	
	if( argc != 3 )
	{
		std::cout << "Please provide input directory and output file name" << std::endl;
		return EXIT_FAILURE;
	}

    char *input_directory = argv[1];
    complex (*datacube)[N_DOP][N_RANGE] = NULL;
    complex (*adaptive_weights)[N_BLOCKS][N_STEERING][N_CHAN*TDOF] = NULL;
    complex (*steering_vectors)[N_CHAN*TDOF] = NULL;
    complex (*output)[N_DOP][N_RANGE] = NULL;

#ifdef ENABLE_CORRECTNESS_CHECKING
    complex (*gold_output)[N_DOP][N_RANGE] = NULL;
#endif

    const size_t num_datacube_elements = N_CHAN * N_DOP * N_RANGE;
    const size_t num_adaptive_weight_elements = N_DOP * N_BLOCKS *
        N_STEERING * (N_CHAN*TDOF);
    const size_t num_steering_vector_elements = N_STEERING *
        (N_CHAN*TDOF);
    const size_t num_output_elements = N_STEERING *
        N_DOP * N_RANGE;
    
    datacube = (complex (*)[N_DOP][N_RANGE])malloc(sizeof(complex) * num_datacube_elements);
    adaptive_weights = (complex (*)[N_BLOCKS][N_STEERING][N_CHAN*TDOF])malloc(sizeof(complex) * num_adaptive_weight_elements);
    steering_vectors = (complex (*)[N_CHAN*TDOF])malloc(sizeof(complex) * num_steering_vector_elements);
    output = (complex (*)[N_DOP][N_RANGE])malloc(sizeof(complex) * num_output_elements);

#ifdef ENABLE_CORRECTNESS_CHECKING
    gold_output = (complex (*)[N_DOP][N_RANGE])malloc(sizeof(complex) * num_output_elements);
#endif

    read_complex_data_file(
        (complex *) datacube,
        input_filename,
        input_directory,
        num_datacube_elements);

    read_complex_data_file(
        (complex *) steering_vectors,
        steering_vector_filename,
        input_directory,
        num_steering_vector_elements);

    read_complex_data_file(
        (complex *) adaptive_weights,
        kernel2_output_filename,
        input_directory,
        num_adaptive_weight_elements);

#ifdef ENABLE_CORRECTNESS_CHECKING
    read_complex_data_file(
        (complex *) gold_output,
        kernel3_output_filename,
        input_directory,
        num_output_elements);
#endif

 	halide_dimension_t datacube_dims[] = {{0, N_CHAN, 1}, {0, N_DOP, 1} , {0, N_RANGE, 1}, {0, 2, 1}};
    Buffer<float> Buffer_datacube( (float*)datacube, 4, datacube_dims);
 	halide_dimension_t weights_dims[] = {{0, N_DOP, 1}, {0, N_BLOCKS, 1}, {0, N_STEERING, 1}, {0, N_CHAN*TDOF, 1}, {0, 2, 1}};
    Buffer<float> Buffer_adaptive_weight( (float*)adaptive_weights, 5, weights_dims);
 	halide_dimension_t steering_dims[] = {{0, N_STEERING, 1}, {0, N_CHAN*TDOF, 1} , {0, 2, 1}};
    Buffer<float> Buffer_steering_vectors( (float*)steering_vectors, 3, steering_dims);
 	halide_dimension_t output_dims[] = {{0, N_STEERING, 1}, {0, N_DOP, 1}, {0, N_RANGE, 1}, {0, 2, 1}};
    Buffer<float> Buffer_output( (float*)output, 4, output_dims);

    // Manually-tuned version
    int timing_iterations = 1;
    double min_t_manual = benchmark(timing_iterations, 10, [&]() {
        inner_product_autoschedule_false_generated(Buffer_datacube, Buffer_adaptive_weight, Buffer_steering_vectors, Buffer_output);
        Buffer_output.device_sync();
    });
    printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);

    // Auto-scheduled version
	// Ben [9/06/20] for some reason the autoscheduled version has an improper access to the output image and fails
    /*double min_t_auto = benchmark(timing_iterations, 10, [&]() {
	inner_product_autoschedule_true_generated(input, r_sigma, output);
        output.device_sync();
    });
    printf("Auto-scheduled time: %gms\n", min_t_auto * 1e3);*/

    //convert_and_save_image(Buffer_output, argv[2]);

#ifdef ENABLE_CORRECTNESS_CHECKING
    {
        double snr;
        printf("\nComputing correctness metrics...\n");
        snr = calculate_snr(
            (complex *) gold_output,
            (complex *) output,
            num_output_elements);
        printf("\tSNR after STAP kernel 3 : %.2f dB\n", snr);
    }
    FREE_AND_NULL(gold_output);
#endif

    FREE_AND_NULL(datacube);
    FREE_AND_NULL(adaptive_weights);
    FREE_AND_NULL(steering_vectors);
    FREE_AND_NULL(output);

    return 0;
}


