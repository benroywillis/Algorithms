#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
//#include "interp1.h"
//#ifndef NO_AUTO_SCHEDULE
//#include "interp1_autoschedule_true_generated.h"
#include "interp1_autoschedule_false_generated.h"
//#endif

#include "HalideBuffer.h"
#include "halide_benchmark.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

// PERFECT related file reading and parameter setting
#include "sar_interp1.h"
#include "sar_utils.h"

#if INPUT_SIZE == INPUT_SIZE_SMALL
    static const char *output_filename = "small_kernel1_output.bin";
    static const char *golden_output_filename = "small_golden_kernel1_output.bin";
    static const char *input_filename = "small_kernel1_input.bin";
#elif INPUT_SIZE == INPUT_SIZE_MEDIUM
    static const char *output_filename = "medium_kernel1_output.bin";
    static const char *golden_output_filename = "medium_golden_kernel1_output.bin";
    static const char *input_filename = "medium_kernel1_input.bin";
#elif INPUT_SIZE == INPUT_SIZE_LARGE
    static const char *output_filename = "large_kernel1_output.bin";
    static const char *golden_output_filename = "large_golden_kernel1_output.bin";
    static const char *input_filename = "large_kernel1_input.bin";
#else
    #error "Unhandled value for INPUT_SIZE"
#endif

static void read_kern1_data_file(
    const char *input_filename,
    const char *directory,
    complex (*data)[N_RANGE],
    double input_start_coords[N_PULSES],
    double input_coord_spacing[N_PULSES],
    double output_coords[PFA_NOUT_RANGE],
    float *window);

int main(int argc, char **argv)
{	
	if( argc != 3 )
	{
		std::cout << "Please provide input directory and output file name" << std::endl;
		return EXIT_FAILURE;
	}
    char* input_directory = argv[1];

    complex (*data)[N_RANGE] = NULL;
    complex (*resampled)[PFA_NOUT_RANGE] = NULL;
    double *input_start_coords = NULL;
    double *input_coord_spacing = NULL;
    double *output_coords = NULL;
    float *window = NULL;
#ifdef ENABLE_CORRECTNESS_CHECKING
    complex (*gold_resampled)[PFA_NOUT_RANGE] = NULL;
#endif

    const size_t num_data_elements = N_PULSES * N_RANGE;
    const size_t num_resampled_elements = N_PULSES * PFA_NOUT_RANGE;
    const size_t num_window_elements = T_PFA;

    data = (complex (*)[N_RANGE])malloc(sizeof(complex) * num_data_elements);
    resampled = (complex (*)[PFA_NOUT_RANGE])malloc(sizeof(complex) * num_resampled_elements);
    input_start_coords = (double*)malloc(sizeof(double) * N_PULSES);
    input_coord_spacing = (double*)malloc(sizeof(double) * N_PULSES);
    output_coords = (double*)malloc(sizeof(double) * PFA_NOUT_RANGE);
    window = (float*)malloc(sizeof(float) * num_window_elements);

#ifdef ENABLE_CORRECTNESS_CHECKING
    gold_resampled = XMALLOC(sizeof(complex) * num_resampled_elements);
#endif

    read_kern1_data_file(
        input_filename,
        input_directory,
        data,
        input_start_coords,
        input_coord_spacing,
        output_coords,
        window);

#ifdef ENABLE_CORRECTNESS_CHECKING
    read_data_file(
        (char *) gold_resampled,
        golden_output_filename,
        input_directory,
        sizeof(complex)*num_resampled_elements);
#endif

 	halide_dimension_t complex_radar_image_dims[] = {{0, N_PULSES, 1}, {0, N_RANGE, 1} , {0, 2, 1}};
    Buffer<double> Buffer_data( (double*)data, 3, complex_radar_image_dims);
 	halide_dimension_t complex_interp1_image_dims[] = {{0, N_PULSES, 1} , {0, PFA_NOUT_RANGE, 1} , {0, 2, 1}};
    Buffer<double> Buffer_resampled( (double*)resampled, 3, complex_interp1_image_dims);
	// the position struct is just 3 doubles x,y,z put together
 	halide_dimension_t vector_PULSES_dims[] = {{0, N_PULSES, 1}};
    Buffer<double> Buffer_input_start_coords( input_start_coords, 1, vector_PULSES_dims );
    Buffer<double> Buffer_input_coord_spacing( input_coord_spacing, 1, vector_PULSES_dims );
 	halide_dimension_t vector_NOUT_RANGE_dims[] = {{0, PFA_NOUT_RANGE, 1}};
    Buffer<double> Buffer_output_coords( output_coords, 1, vector_NOUT_RANGE_dims );
 	halide_dimension_t vector_PFA_dims[] = {{0, T_PFA, 1}};
    Buffer<float> Buffer_window( window, 1, vector_PFA_dims);

    // Manually-tuned version
    int timing_iterations = 15;
    double min_t_manual = benchmark(timing_iterations, 10, [&]() {
        interp1_autoschedule_false_generated(Buffer_data, Buffer_window, Buffer_input_start_coords, Buffer_input_coord_spacing, Buffer_output_coords, Buffer_resampled);
        Buffer_resampled.device_sync();
    });
    printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);

    // Auto-scheduled version
	// Ben [9/06/20] for some reason the autoscheduled version has an improper access to the output image and fails
    /*double min_t_auto = benchmark(timing_iterations, 10, [&]() {
	interp1_autoschedule_true_generated(input, r_sigma, output);
        output.device_sync();
    });
    printf("Auto-scheduled time: %gms\n", min_t_auto * 1e3);*/

#ifdef ENABLE_CORRECTNESS_CHECKING
    {
        double snr = calculate_snr(
            (complex *) gold_resampled,
            (complex *) resampled,
            num_resampled_elements);
        printf("\nImage correctness SNR = %.2f\n", snr);
    }
#endif

    convert_and_save_image(Buffer_resampled, argv[2]);
	free(data);
	free(resampled);
	free(input_start_coords);
	free(input_coord_spacing);
	free(output_coords);
	free(window);
#ifdef ENABLE_CORRECTNESS_CHECKING
    	FREE_AND_NULL(gold_resampled);
#endif
    return 0;
}

void read_kern1_data_file(
    const char *input_filename,
    const char *directory,
    complex (*data)[N_RANGE],
    double input_start_coords[N_PULSES],
    double input_coord_spacing[N_PULSES],
    double output_coords[PFA_NOUT_RANGE],
    float *window)
{
    FILE *fp = NULL;
    const size_t num_data_elements = N_RANGE*N_PULSES;
    const size_t num_window_elements = T_PFA;
    size_t n;

    char dir_and_filename[MAX_DIR_AND_FILENAME_LEN];

    assert(input_filename != NULL);
    assert(directory != NULL);
    assert(data != NULL);
    assert(input_start_coords != NULL);
    assert(input_coord_spacing != NULL);
    assert(output_coords != NULL);
    assert(window != NULL);

    concat_dir_and_filename(
        dir_and_filename,
        directory,
        input_filename);

    fp = fopen(dir_and_filename, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: Unable to open input file %s for reading.\n",
            dir_and_filename);
        exit(EXIT_FAILURE);
    }

    if ((n = fread(data, sizeof(complex), num_data_elements, fp)) !=
        num_data_elements)
    {
        fprintf(stderr, "Error: Unable to read phase history data from %s "
            "(read %lu elements instead of %lu).\n",
            input_filename, n, num_data_elements);
        exit(EXIT_FAILURE);
    }

    if ((n = fread(input_start_coords, sizeof(double), N_PULSES, fp)) !=
        N_PULSES)
    {
        fprintf(stderr, "Error: Unable to read input start coordinates from %s "
            "(read %lu elements instead of %d).\n",
            input_filename, n, N_PULSES);
        exit(EXIT_FAILURE);
    }

    if ((n = fread(input_coord_spacing, sizeof(double), N_PULSES, fp)) !=
        N_PULSES)
    {
        fprintf(stderr, "Error: Unable to read input coordinate spacing from %s "
            "(read %lu elements instead of %d).\n",
            input_filename, n, N_PULSES);
        exit(EXIT_FAILURE);
    }

    if ((n = fread(output_coords, sizeof(double), PFA_NOUT_RANGE, fp)) !=
        PFA_NOUT_RANGE)
    {
        fprintf(stderr, "Error: Unable to read output coordinates from %s "
            "(read %lu elements instead of %d).\n",
            input_filename, n, PFA_NOUT_RANGE);
        exit(EXIT_FAILURE);
    }

    if ((n = fread(window, sizeof(float), num_window_elements, fp)) !=
        num_window_elements)
    {
        fprintf(stderr, "Error: Unable to read window values from %s "
            "(read %lu elements instead of %lu).\n",
            input_filename, n, num_window_elements);
        exit(EXIT_FAILURE);
    }

    fclose(fp);
}
