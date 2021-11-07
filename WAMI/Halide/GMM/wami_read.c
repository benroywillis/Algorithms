/* -*-Mode: C;-*- */

/**BeginCopyright************************************************************
 *
 * $HeadURL$
 * $Id: 11dcfc83dec2b02b0c1cde73d71def759ccc0525 $
 *
 *---------------------------------------------------------------------------
 * Part of PERFECT Benchmark Suite (hpc.pnnl.gov/projects/PERFECT/)
 *---------------------------------------------------------------------------
 *
 * Copyright ((c)) 2014, Battelle Memorial Institute
 * Copyright ((c)) 2014, Georgia Tech Research Corporation
 * All rights reserved.
 *
 * 1. Battelle Memorial Institute (hereinafter Battelle) and Georgia Tech
 *    Research Corporation (GTRC) hereby grant permission to any person
 *    or entity lawfully obtaining a copy of this software and associated
 *    documentation files (hereinafter "the Software") to redistribute
 *    and use the Software in source and binary forms, with or without
 *    modification.  Such person or entity may use, copy, modify, merge,
 *    publish, distribute, sublicense, and/or sell copies of the
 *    Software, and may permit others to do so, subject to the following
 *    conditions:
 * 
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimers.
 * 
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in
 *      the documentation and/or other materials provided with the
 *      distribution.
 * 
 *    * Other than as used herein, neither the name Battelle Memorial
 *      Institute nor Battelle may be used in any form whatsoever without
 *      the express written consent of Battelle.
 * 
 *      Other than as used herein, neither the name Georgia Tech Research
 *      Corporation nor GTRC may not be used in any form whatsoever
 *      without the express written consent of GTRC.
 * 
 *    * Redistributions of the software in any form, and publications
 *      based on work performed using the software should include the
 *      following citation as a reference:
 * 
 *      Kevin Barker, Thomas Benson, Dan Campbell, David Ediger, Roberto
 *      Gioiosa, Adolfy Hoisie, Darren Kerbyson, Joseph Manzano, Andres
 *      Marquez, Leon Song, Nathan R. Tallent, and Antonino Tumeo.
 *      PERFECT (Power Efficiency Revolution For Embedded Computing
 *      Technologies) Benchmark Suite Manual. Pacific Northwest National
 *      Laboratory and Georgia Tech Research Institute, December 2013.
 *      http://hpc.pnnl.gov/projects/PERFECT/
 *
 * 2. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 *    BATTELLE, GTRC, OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 *    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 *    OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 **EndCopyright*************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "wami_params.h"
#include "wami_utils.h"
#include "wami_gmm.h"
#include "wami_morpho.h"

void read_gmm_input_data(
    float mu[WAMI_GMM_IMG_NUM_ROWS][WAMI_GMM_IMG_NUM_COLS][WAMI_GMM_NUM_MODELS],
    float sigma[WAMI_GMM_IMG_NUM_ROWS][WAMI_GMM_IMG_NUM_COLS][WAMI_GMM_NUM_MODELS],
    float weights[WAMI_GMM_IMG_NUM_ROWS][WAMI_GMM_IMG_NUM_COLS][WAMI_GMM_NUM_MODELS],
    u16 frames[WAMI_GMM_NUM_FRAMES][WAMI_GMM_IMG_NUM_ROWS][WAMI_GMM_IMG_NUM_COLS],
    const char *filename,
    const char *directory)
{
    char dir_and_filename[MAX_DIR_AND_FILENAME_LEN];
    FILE *fp = NULL;
    const size_t num_model_param_bytes = WAMI_GMM_IMG_NUM_ROWS * WAMI_GMM_IMG_NUM_COLS *
        WAMI_GMM_NUM_MODELS * sizeof(float);
    const size_t num_image_bytes = WAMI_GMM_IMG_NUM_ROWS * WAMI_GMM_IMG_NUM_COLS *
        sizeof(u16);
    size_t nread = 0;
    int success = 0, i;
    u16 width, height, channels, depth;

    assert(filename != NULL);
    assert(directory != NULL);

    concat_dir_and_filename(
        dir_and_filename,
        directory,
        filename);

    fp = fopen(dir_and_filename, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: Unable to open input file %s for reading.\n",
            dir_and_filename);
        exit(EXIT_FAILURE);
    }

    success = (fread(mu, 1, num_model_param_bytes, fp) == num_model_param_bytes);
    success &= (fread(sigma, 1, num_model_param_bytes, fp) == num_model_param_bytes);
    success &= (fread(weights, 1, num_model_param_bytes, fp) == num_model_param_bytes);
    if (! success)
    {
        fprintf(stderr, "Error: Unable to read model parameters from %s.\n",
            dir_and_filename);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < WAMI_GMM_NUM_FRAMES; ++i)
    {
        success = (fread(&width, sizeof(u16), 1, fp) == 1);
        success &= (fread(&height, sizeof(u16), 1, fp) == 1);
        success &= (fread(&channels, sizeof(u16), 1, fp) == 1);
        success &= (fread(&depth, sizeof(u16), 1, fp) == 1);
        if (! success)
        {
            fprintf(stderr, "Error: Unable to read image %d header from %s.\n",
                i, dir_and_filename);
            exit(EXIT_FAILURE);
        }

        if (width != WAMI_GMM_IMG_NUM_COLS || height != WAMI_GMM_IMG_NUM_ROWS ||
            channels != 1 || depth != 2)
        {
            fprintf(stderr, "Error: Mismatch for image header %d in %s: "
                "[width,height,channels,depth] = [%u,%u,%u,%u].\n",
                i, dir_and_filename, width, height, channels, depth);
            exit(EXIT_FAILURE);
        }

        nread = fread(&frames[i][0][0], 1, num_image_bytes, fp);
        if (nread != num_image_bytes)
        {
            fprintf(stderr, "Error: Unable to read input image %d from %s.\n",
                i, dir_and_filename);
            exit(EXIT_FAILURE);
        } 
    }

    fclose(fp);
}
