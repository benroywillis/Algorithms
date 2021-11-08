#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
//#include "interp2.h"
//#ifndef NO_AUTO_SCHEDULE
//#include "interp2_autoschedule_true_generated.h"
#include "interp2_autoschedule_false_generated.h"
//#endif

#include "HalideBuffer.h"
#include "halide_benchmark.h"
#include "halide_image_io.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;

// PERFECT related file reading and parameter setting
#include "sar_interp2.h"
#include "sar_utils.h"
#include "sar_params.h"
#include "sar_utils.h"
#include "sar_interp2.h"

#define ENABLE_CORRECTNESS_CHECKING

#if INPUT_SIZE == INPUT_SIZE_SMALL
    static const char *output_filename = "small_kernel2_output.bin";
    static const char *golden_output_filename = "small_golden_kernel2_output.bin";
    static const char *input_filename = "small_kernel2_input.bin";
#elif INPUT_SIZE == INPUT_SIZE_MEDIUM
    static const char *output_filename = "medium_kernel2_output.bin";
    static const char *golden_output_filename = "medium_golden_kernel2_output.bin";
    static const char *input_filename = "medium_kernel2_input.bin";
#elif INPUT_SIZE == INPUT_SIZE_LARGE
    static const char *output_filename = "large_kernel2_output.bin";
    static const char *golden_output_filename = "large_golden_kernel2_output.bin";
    static const char *input_filename = "large_kernel2_input.bin";
#else
    #error "Unhandled value for INPUT_SIZE"
#endif

#define WRITE_OUTPUT_TO_DISK

static void read_kern2_data_file(
    const char *input_filename,
    const char *directory,
    complex (*data)[PFA_NOUT_RANGE],
    double *input_coords,
    double *output_coords,
    float *window);

int main(int argc, char **argv)
{
	if( argc != 3 )
	{
		std::cout << "Please provide input directory and output file name" << std::endl;
		return EXIT_FAILURE;
	}
    char* input_directory = argv[1];

    complex (*data)[PFA_NOUT_RANGE] = NULL;
    complex (*resampled)[PFA_NOUT_RANGE] = NULL;
    double (*input_coords)[N_PULSES] = NULL;
    double *output_coords = NULL;
    float *window = NULL;
#ifdef ENABLE_CORRECTNESS_CHECKING
    complex (*gold_resampled)[PFA_NOUT_RANGE] = NULL;
#endif

    const size_t num_data_elements = N_PULSES * PFA_NOUT_RANGE;
    const size_t num_resampled_elements = PFA_NOUT_AZIMUTH * PFA_NOUT_RANGE;
    const size_t num_window_elements = T_PFA;

    data = (complex (*)[PFA_NOUT_RANGE])malloc(sizeof(complex) * num_data_elements);
    resampled = (complex (*)[PFA_NOUT_RANGE])malloc(sizeof(complex) * num_resampled_elements);
    input_coords = (double (*)[N_PULSES])malloc(sizeof(double) * PFA_NOUT_RANGE * N_PULSES);
    output_coords = (double*)malloc(sizeof(double) * PFA_NOUT_RANGE);
    window = (float*)malloc(sizeof(float) * num_window_elements);

    read_kern2_data_file(
        input_filename,
        input_directory,
        data,
        (double *) input_coords,
        output_coords,
        window);

 	halide_dimension_t complex_radar_image_dims[] = {{0, N_PULSES, 1}, {0, PFA_NOUT_RANGE, 1} , {0, 2, 1}};
    Buffer<double> Buffer_data( (double*)data, 3, complex_radar_image_dims);
 	halide_dimension_t complex_interp2_image_dims[] = {{0, PFA_NOUT_AZIMUTH, 1} , {0, PFA_NOUT_RANGE, 1} , {0, 2, 1}};
    Buffer<double> Buffer_resampled( (double*)resampled, 3, complex_interp2_image_dims);
	// the position struct is just 3 doubles x,y,z put together
 	halide_dimension_t vector_PULSES_dims[] = {{0, PFA_NOUT_RANGE, 1}, {0, N_PULSES, 1}};
    Buffer<double> Buffer_input_coords( (double*)input_coords, 2, vector_PULSES_dims );
 	halide_dimension_t vector_NOUT_RANGE_dims[] = {{0, PFA_NOUT_RANGE, 1}};
    Buffer<double> Buffer_output_coords( output_coords, 1, vector_NOUT_RANGE_dims );
 	halide_dimension_t vector_PFA_dims[] = {{0, T_PFA, 1}};
    Buffer<float> Buffer_window( window, 1, vector_PFA_dims);

    // Manually-tuned version
    int timing_iterations = 15;
    double min_t_manual = benchmark(timing_iterations, 10, [&]() {
        interp2_autoschedule_false_generated(Buffer_data, Buffer_window, Buffer_input_coords, Buffer_output_coords, Buffer_resampled);
        Buffer_resampled.device_sync();
    });
    printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);

    // Auto-scheduled version
	// Ben [9/06/20] for some reason the autoscheduled version has an improper access to the output image and fails
    /*double min_t_auto = benchmark(timing_iterations, 10, [&]() {
	interp2_autoschedule_true_generated(input, r_sigma, output);
        output.device_sync();
    });
    printf("Auto-scheduled time: %gms\n", min_t_auto * 1e3);*/

    convert_and_save_image(Buffer_resampled, argv[2]);
	free(data);
	free(resampled);
	free(input_coords);
	free(output_coords);
	free(window);
    return 0;
}

void read_kern2_data_file(
    const char *input_filename,
    const char *input_directory,
    complex (*data)[PFA_NOUT_RANGE],
    double *input_coords,
    double *output_coords,
    float *window)
{
    FILE *fp = NULL;
    const size_t num_data_elements = PFA_NOUT_RANGE*N_PULSES;
    const size_t num_window_elements = T_PFA;
    char dir_and_filename[MAX_DIR_AND_FILENAME_LEN];
    size_t n;

    concat_dir_and_filename(
        dir_and_filename,
        input_directory,
        input_filename);

    fp = fopen(dir_and_filename, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: Unable to open input filename %s.\n",
            input_filename);
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

    if ((n = fread(input_coords, sizeof(double), N_PULSES * PFA_NOUT_RANGE, fp)) !=
        N_PULSES * PFA_NOUT_RANGE)
    {
        fprintf(stderr, "Error: Unable to read input coordinates from %s "
            "(read %lu elements instead of %d).\n",
            input_filename, n, N_PULSES * PFA_NOUT_RANGE);
        exit(EXIT_FAILURE);
    }

    if ((n = fread(output_coords, sizeof(double), PFA_NOUT_AZIMUTH, fp)) !=
        PFA_NOUT_AZIMUTH)
    {
        fprintf(stderr, "Error: Unable to read output coordinates from %s "
            "(read %lu elements instead of %d).\n",
            input_filename, n, PFA_NOUT_AZIMUTH);
        exit(EXIT_FAILURE);
    }

    if ((n = fread(window, sizeof(float), num_window_elements, fp)) !=
        num_window_elements)
    {
        fprintf(stderr, "Error: Unable to read window values from %s "
            "(read %lu elements instead of %lu;).\n",
            input_filename, n, num_window_elements);
        exit(EXIT_FAILURE);
    }

    fclose(fp);
}
