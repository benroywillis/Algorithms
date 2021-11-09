#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
//#include "bp.h"
//#ifndef NO_AUTO_SCHEDULE
//#include "bp_autoschedule_true_generated.h"
#include "bp_autoschedule_false_generated.h"
//#endif

#include "HalideBuffer.h"
#include "halide_benchmark.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

// PERFECT related file reading and parameter setting
#include "sar_backprojection.h"
#include "sar_utils.h"

#if INPUT_SIZE == INPUT_SIZE_SMALL
    static const char *output_filename = "small_kernel3_output.bin";
    static const char *golden_output_filename = "small_golden_kernel3_output.bin";
    static const char *input_filename = "small_kernel3_input.bin";
#elif INPUT_SIZE == INPUT_SIZE_MEDIUM
    static const char *output_filename = "medium_kernel3_output.bin";
    static const char *golden_output_filename = "medium_golden_kernel3_output.bin";
    static const char *input_filename = "medium_kernel3_input.bin";
#elif INPUT_SIZE == INPUT_SIZE_LARGE
    static const char *output_filename = "large_kernel3_output.bin";
    static const char *golden_output_filename = "large_golden_kernel3_output.bin";
    static const char *input_filename = "large_kernel3_input.bin";
#else
    #error "Unhandled value for INPUT_SIZE"
#endif

static void read_bp_data_file(
    const char *input_filename,
    const char *input_directory,
    complex (*upsampled_data)[N_RANGE_UPSAMPLED],
    position *platpos,
    double *fc,
    double *R0,
    double *dR);

int main(int argc, char **argv)
{	
	if( argc != 3 )
	{
		std::cout << "Please provide input directory and output file name" << std::endl;
		return EXIT_FAILURE;
	}

    complex (*data)[N_RANGE_UPSAMPLED] = NULL;
    complex (*image)[BP_NPIX_X] = NULL;
    const char *input_directory = NULL;
    position *platpos = NULL;
#ifdef ENABLE_CORRECTNESS_CHECKING
    complex (*gold_image)[BP_NPIX_X] = NULL;
#endif

    const size_t num_data_elements = N_PULSES * N_RANGE_UPSAMPLED;
    const size_t num_image_elements = BP_NPIX_Y * BP_NPIX_X;

    double z0 = 0.0;
    double fc, R0, dR, dxdy, ku;

    input_directory = argv[1];
    
    data = (complex (*)[N_RANGE_UPSAMPLED])malloc(sizeof(complex) * num_data_elements);
    image = (complex (*)[BP_NPIX_X])malloc(sizeof(complex) * num_image_elements);
    platpos = (position*)malloc(sizeof(position) * N_PULSES);
#ifdef ENABLE_CORRECTNESS_CHECKING
    gold_image = (complex (*) [BP_NPIX_X])malloc(sizeof(complex) * num_image_elements);
#endif

    read_bp_data_file(
        input_filename,
        input_directory,
        data,
        platpos,
        &fc,
        &R0,
        &dR);
#ifdef ENABLE_CORRECTNESS_CHECKING
    read_data_file(
        (char *) gold_image,
        golden_output_filename,
        input_directory,
        sizeof(complex)*num_image_elements);
#endif

    dxdy = dR;
    dR /= RANGE_UPSAMPLE_FACTOR;
    ku = 2.0 * M_PI * fc / SPEED_OF_LIGHT;

 	halide_dimension_t complex_radar_image_dims[] = {{0, N_PULSES, 1}, {0, N_RANGE_UPSAMPLED, 1} , {0, 2, 1}};
    Buffer<double> Buffer_image( (double*)image, 3, complex_radar_image_dims);
 	halide_dimension_t complex_bp_image_dims[] = {{0, BP_NPIX_Y, 1} , {0, BP_NPIX_X, 1} , {0, 2, 1}};
    Buffer<double> Buffer_data( (double*)data, 3, complex_bp_image_dims);
	// the position struct is just 3 doubles x,y,z put together
 	halide_dimension_t position_dims[] = {{0, N_PULSES, 1},{0, 3, 1}};
    Buffer<double> Buffer_position( (double*)platpos, 2, position_dims);
 	halide_dimension_t scalar_dims[] = {{0, 1, 1}};
    Buffer<double> Buffer_ku( &ku, 1, scalar_dims);
    Buffer<double> Buffer_R0( &R0, 1, scalar_dims);
    Buffer<double> Buffer_dR( &dR, 1, scalar_dims);
    Buffer<double> Buffer_dxdy( &dxdy, 1, scalar_dims);
    Buffer<double> Buffer_z0( &z0, 1, scalar_dims);

    // Manually-tuned version
    int timing_iterations = 15;
    double min_t_manual = benchmark(timing_iterations, 10, [&]() {
        bp_autoschedule_false_generated(Buffer_data, Buffer_position, Buffer_ku, Buffer_R0, Buffer_dR, Buffer_dxdy, Buffer_z0, Buffer_image);
        Buffer_image.device_sync();
    });
    printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);

    // Auto-scheduled version
	// Ben [9/06/20] for some reason the autoscheduled version has an improper access to the output image and fails
    /*double min_t_auto = benchmark(timing_iterations, 10, [&]() {
	bp_autoschedule_true_generated(input, r_sigma, output);
        output.device_sync();
    });
    printf("Auto-scheduled time: %gms\n", min_t_auto * 1e3);*/

    convert_and_save_image(Buffer_image, argv[2]);

#ifdef ENABLE_CORRECTNESS_CHECKING
    {
        double snr = calculate_snr(
            (complex *) gold_image,
            (complex *) image,
            num_image_elements);
        printf("\nImage correctness SNR: %.2f\n", snr);
    }
#endif

	free(data);
	free(image);
	free(platpos);
#ifdef ENABLE_CORRECTNESS_CHECKING
    free(gold_image);
#endif

    return 0;
}

void read_bp_data_file(
    const char *input_filename,
    const char *input_directory,
    complex (*upsampled_data)[N_RANGE_UPSAMPLED],
    position *platpos,
    double *fc,
    double *R0,
    double *dR)
{
    FILE *fp = NULL;
    const size_t num_data_elements = N_RANGE_UPSAMPLED*N_PULSES;
    char dir_and_filename[MAX_DIR_AND_FILENAME_LEN];
    size_t n;

    assert(input_filename != NULL);
    assert(input_directory != NULL);
    assert(upsampled_data != NULL);
    assert(platpos != NULL);
    assert(fc != NULL);
    assert(R0 != NULL);
    assert(dR != NULL);

    concat_dir_and_filename(
        dir_and_filename,
        input_directory,
        input_filename);

    fp = fopen(dir_and_filename, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: Unable to open input file %s.\n",
            input_filename);
        exit(EXIT_FAILURE);
    }

    if (fread(fc, sizeof(double), 1, fp) != 1)
    {
        fprintf(stderr, "Error: Unable to read parameter fc from %s.\n",
            input_filename);
        exit(EXIT_FAILURE);
    }

    if (fread(R0, sizeof(double), 1, fp) != 1)
    {
        fprintf(stderr, "Error: Unable to read parameter R0 from %s.\n",
            input_filename);
        exit(EXIT_FAILURE);
    }

    if (fread(dR, sizeof(double), 1, fp) != 1)
    {
        fprintf(stderr, "Error: Unable to read parameter dR from %s.\n",
            input_filename);
        exit(EXIT_FAILURE);
    }

    if (fread(platpos, sizeof(position), N_PULSES, fp) != N_PULSES)
    {
        fprintf(stderr, "Error: Unable to read platform positions from %s.\n",
            input_filename);
        exit(EXIT_FAILURE);
    }

    if ((n = fread(upsampled_data, sizeof(complex), num_data_elements, fp)) !=
        num_data_elements)
    {
        fprintf(stderr, "Error: Unable to read phase history data from %s.\n",
            input_filename);
        exit(EXIT_FAILURE);
    }

    fclose(fp);
}
