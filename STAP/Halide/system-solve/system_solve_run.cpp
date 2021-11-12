#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
//#include "system_solve.h"
//#ifndef NO_AUTO_SCHEDULE
//#include "system_solve_autoschedule_true_generated.h"
#include "system_solve_autoschedule_false_generated.h"
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
    static const char *kernel1_output_filename = "small_kernel1_output.bin";
    static const char *kernel2_output_filename = "small_kernel2_output.bin";
    static const char *steering_vector_filename = "small_steering_vectors.bin";
#elif INPUT_SIZE == INPUT_SIZE_MEDIUM
    static const char *kernel1_output_filename = "medium_kernel1_output.bin";
    static const char *kernel2_output_filename = "medium_kernel2_output.bin";
    static const char *steering_vector_filename = "medium_steering_vectors.bin";
#elif INPUT_SIZE == INPUT_SIZE_LARGE
    static const char *kernel1_output_filename = "large_kernel1_output.bin";
    static const char *kernel2_output_filename = "large_kernel2_output.bin";
    static const char *steering_vector_filename = "large_steering_vectors.bin";
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

    complex (*covariances)[N_BLOCKS][N_CHAN*TDOF][N_CHAN*TDOF] = NULL;
    complex (*cholesky_factors)[N_BLOCKS][N_CHAN*TDOF][N_CHAN*TDOF] = NULL;
    complex (*adaptive_weights)[N_BLOCKS][N_STEERING][N_CHAN*TDOF] = NULL;
    complex (*steering_vectors)[N_CHAN*TDOF] = NULL;
    char *input_directory = argv[1];

#ifdef ENABLE_CORRECTNESS_CHECKING
    complex (*gold_weights)[N_BLOCKS][N_STEERING][N_CHAN*TDOF] = NULL;
#endif

    const size_t num_covariance_elements = (TDOF*N_CHAN) * (TDOF*N_CHAN) *
        N_DOP * N_BLOCKS;
    const size_t num_adaptive_weight_elements = N_DOP * N_BLOCKS *
        N_STEERING * (N_CHAN*TDOF);
    const size_t num_steering_vector_elements = N_STEERING *
        (N_CHAN*TDOF);

    covariances = (complex (*)[N_BLOCKS][TDOF*N_CHAN][TDOF*N_CHAN])malloc(sizeof(complex) * num_covariance_elements);
    cholesky_factors = (complex (*)[N_BLOCKS][TDOF*N_CHAN][TDOF*N_CHAN])malloc(sizeof(complex) * num_covariance_elements);
    adaptive_weights = (complex (*)[N_BLOCKS][N_STEERING][N_CHAN*TDOF])malloc(sizeof(complex) * num_adaptive_weight_elements);
    steering_vectors = (complex (*)[N_CHAN*TDOF])malloc(sizeof(complex) * num_steering_vector_elements);

#ifdef ENABLE_CORRECTNESS_CHECKING
    gold_weights = (complex (*)[N_BLOCKS][N_STEERING][N_CHAN*TDOF])malloc(sizeof(complex) * num_adaptive_weight_elements);
#endif

    printf("Loading input files from %s...\n", input_directory);

    read_complex_data_file(
        (complex *) covariances,
        kernel1_output_filename,
        input_directory,
        num_covariance_elements);

    read_complex_data_file(
        (complex *) steering_vectors,
        steering_vector_filename,
        input_directory,
        num_steering_vector_elements);

#ifdef ENABLE_CORRECTNESS_CHECKING
    read_complex_data_file(
        (complex *) gold_weights,
        kernel2_output_filename,
        input_directory,
        num_adaptive_weight_elements);
#endif

    memset(adaptive_weights, 0, sizeof(complex) * num_adaptive_weight_elements);

 	//halide_dimension_t covariance_dims[] = {{0, N_DOP, 1}, {0, N_BLOCKS, 1}, {0, TDOF*N_CHAN, 1}, {0, TDOF*N_CHAN, 1}, {0, 2, 1}};
    Buffer<float> Buffer_covariances( (float*)covariances, 2, TDOF*N_CHAN, TDOF*N_CHAN, N_BLOCKS, N_DOP);
    Buffer<float> Buffer_cholesky_factors( (float*)cholesky_factors, 2, TDOF*N_CHAN, TDOF*N_CHAN, N_BLOCKS, N_DOP);
 	//halide_dimension_t steering_dims[] = {{0, N_STEERING, 1}, {0, N_CHAN*TDOF, 1}, {0, 2, 1}};
    Buffer<float> Buffer_steering_vectors( (float*)steering_vectors, 2, N_CHAN*TDOF, N_STEERING);
 	//halide_dimension_t adaptive_weights_dims[] = {{0, N_DOP, 1}, {0, N_BLOCKS, 1},{0, N_STEERING, 1}, {0, TDOF*N_CHAN, 1}, {0, 2, 1}};
    Buffer<float> Buffer_adaptive_weights( (float*)adaptive_weights, 2, N_CHAN*TDOF, N_STEERING, N_BLOCKS, N_DOP);
	uint64_t badMatch = 0;
	for( int i = 0; i < N_DOP; i++ )
	{
		for( int j = 0; j < N_BLOCKS; j++ )
		{
			for( int k = 0; k < N_CHAN*TDOF; k++ )
			{
				for( int l = 0; l < N_CHAN*TDOF; l++ )
				{
					if( covariances[i][j][k][l].re != Buffer_covariances(0, l, k, j, i) || ( covariances[i][j][k][l].im != Buffer_covariances(1, l, k, j, i) ) )
					{
					//printf("%.2f + j%.2f , %.2f + j%.2f\n", 
														//datacube[i][j][k].re, 
														//datacube[i][j][k].im, 
														//Buffer_datacube(0, k, j, i), 
														//Buffer_datacube(1, k, j, i)); 
						badMatch++;
					}
				}
			}
		}
	}
	printf(" %.2f%% of the inputs did not match\n", (float)( (float)badMatch / (float)(N_DOP*N_BLOCKS*N_CHAN*TDOF*N_CHAN*TDOF) * 100));

    // Manually-tuned version
    int timing_iterations = 1;
    double min_t_manual = benchmark(timing_iterations, 10, [&]() {
        system_solve_autoschedule_false_generated(Buffer_covariances, Buffer_cholesky_factors, Buffer_steering_vectors, Buffer_adaptive_weights);
        Buffer_adaptive_weights.device_sync();
    });

	badMatch = 0;
	for( int i = 0; i < N_DOP; i++ )
	{
		for( int j = 0; j < N_BLOCKS; j++ )
		{
			for( int k = 0; k < N_STEERING; k++ )
			{
				for( int l = 0; l < N_CHAN*TDOF; l++ )
				{
					if( gold_weights[i][j][k][l].re != Buffer_adaptive_weights(0, l, k, j, i) || ( gold_weights[i][j][k][l].im != Buffer_adaptive_weights(1, l, k, j, i) ) )
					{
						//printf("%.2f + j%.2f , %.2f, %.2f\n", 
														  //Buffer_covariances(0, l, k, j, i), 
														  //Buffer_covariances(1, l, k, j, i), 
														  //gold_covariances[i][j][k][l].re,
														  //gold_covariances[i][j][k][l].im);
						badMatch++;
					}
				}
			}
		}
	}
	printf(" %.2f%% of the outputs did not match\n", (float)( (float)badMatch / (float)(N_DOP*N_BLOCKS*N_STEERING*N_CHAN*TDOF) *100 ));
    printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);

    // Auto-scheduled version
	// Ben [9/06/20] for some reason the autoscheduled version has an improper access to the outer image and fails
    /*double min_t_auto = benchmark(timing_iterations, 10, [&]() {
	system_solve_autoschedule_true_generated(input, r_sigma, outer);
        outer.device_sync();
    });
    printf("Auto-scheduled time: %gms\n", min_t_auto * 1e3);*/

    //convert_and_save_image(Buffer_adaptive_weights, argv[2]);

#ifdef ENABLE_CORRECTNESS_CHECKING
    {
        double snr;
        printf("Computing correctness metrics...\n");
        snr = calculate_snr(
            (complex *) gold_weights,
            (complex *) adaptive_weights,
            num_adaptive_weight_elements);
        printf("\tSNR after STAP kernel 2 : %.2f dB\n", snr);
    }
    FREE_AND_NULL(gold_weights);
#endif

    FREE_AND_NULL(covariances);
    FREE_AND_NULL(cholesky_factors);
    FREE_AND_NULL(adaptive_weights);
    FREE_AND_NULL(steering_vectors);

    return 0;
}


