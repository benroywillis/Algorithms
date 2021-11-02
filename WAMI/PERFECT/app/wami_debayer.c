/* -*-Mode: C;-*- */

/**BeginCopyright************************************************************
 *
 * $HeadURL$
 * $Id: 2629914133caed8e93ba4849b45ee57f2509c1bf $
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

#include "wami_debayer.h"

#define PIXEL_MAX 65535

static u16 compute_and_clamp_pixel(
    u16 pos,
    u16 neg)
{
    if (pos < neg)
    {
        return 0;
    }
    else
    {
        const u16 pixel = (pos - neg) >> 3;
        if (pixel > PIXEL_MAX) { return PIXEL_MAX; }
        else { return pixel; }
    }
}

/*
 * This version handles masks with fractional negative values. In those
 * cases truncating before subtraction does not generally yield the
 * same result as truncating after subtraction.  The negative value
 * is using weights in units of 1/16ths so that the one-half portions
 * are retained.
 */
static u16 compute_and_clamp_pixel_fractional_neg(
    u16 pos,
    u16 neg)
{
    /*
     * The positive portion is converted to u32 prior to doubling because
     * otherwise some of the weights could yield overflow. At that point,
     * all weights are effectively 16x their actual value, so combining
     * the positive and negative portions and then shifting by four bits
     * yields the equivalent of a floor() applied to the result of the
     * full precision convolution.
     */
    const u32 pos_u32 = ((u32) pos) << 1;
    const u32 neg_u32 = (u32) neg;
    if (pos_u32 < neg_u32)
    {
        return 0;
    }
    else
    {
        const u16 pixel = (u16) ((pos_u32 - neg_u32) >> 4);
        if (pixel > PIXEL_MAX) { return PIXEL_MAX; }
        else { return pixel; }
    }
}

static u16 interp_G_at_RRR_or_G_at_BBB(
    u16 * const bayer,
    u32 row,
    u32 col,
    u32 nrows,
    u32 ncols)
{
    /*
     * The mask to interpolate G at R or B is:
     *
     * [0  0 -1  0  0
     *  0  0  2  0  0
     * -1  2  4  2 -1
     *  0  0  2  0  0
     *  0  0 -1  0  0] /8
     */
    const u16 pos =
         2 * bayer[ncols*(row-1) + (col)] +
         2 * bayer[ncols*(row) + (col-1)] +
         4 * bayer[ncols*(row) + (col)] +
         2 * bayer[ncols*(row) + (col+1)] +
         2 * bayer[ncols*(row+1) + (col)];
    const u16 neg =
             bayer[ncols*(row) + (col+2)] +
             bayer[ncols*(row-2) + (col)] +
             bayer[ncols*(row) + (col-2)] +
             bayer[ncols*(row+2) + (col)];

    return compute_and_clamp_pixel(pos, neg);
}

static u16 interp_R_at_GRB_or_B_at_GBR(
    u16 * const bayer,
    u32 row,
    u32 col,
    u32 nrows,
    u32 ncols)
{
    /*
     * [0  0 0.5 0  0
     *  0 -1  0 -1  0
     * -1  4  5  4 -1
     *  0 -1  0 -1  0
     *  0  0 0.5 0  0] /8;
     */
    const u16 pos =
          ((bayer[ncols*(row-2) + (col)] + bayer[ncols*(row+2) + (col)]) >> 1) +
        4 * bayer[ncols*(row) + (col-1)] +
        5 * bayer[ncols*(row) + (col)] +
        4 * bayer[ncols*(row) + (col+1)];
    const u16 neg =
            bayer[ncols*(row-1) + (col-1)] +
            bayer[ncols*(row-1) + (col+1)] +
            bayer[ncols*(row) + (col-2)] +
            bayer[ncols*(row) + (col+2)] +
            bayer[ncols*(row+1) + (col-1)] +
            bayer[ncols*(row+1) + (col+1)];

    return compute_and_clamp_pixel(pos, neg);
}
    
static u16 interp_R_at_GBR_or_B_at_GRB(
    u16 * const bayer,
    u32 row,
    u32 col,
    u32 nrows,
    u32 ncols)
{
    /*
     * [0  0 -1  0  0
     *  0 -1  4 -1  0
     * 0.5 0  5  0 0.5
     *  0 -1  4 -1  0
     *  0  0 -1  0  0] /8;
     */
    const u16 pos =
        4 * bayer[ncols*(row-1) + (col)] +
          ((bayer[ncols*(row) + (col-2)] + bayer[ncols*(row) + (col+2)]) >> 1) +
        5 * bayer[ncols*(row) + (col)] +
        4 * bayer[ncols*(row+1) + (col)];
    const u16 neg =
            bayer[ncols*(row-2) + (col)] +
            bayer[ncols*(row-1) + (col-1)] +
            bayer[ncols*(row-1) + (col+1)] +
            bayer[ncols*(row+1) + (col-1)] +
            bayer[ncols*(row+1) + (col+1)] +
            bayer[ncols*(row+2) + (col)];

    return compute_and_clamp_pixel(pos, neg);
}

static u16 interp_R_at_BBB_or_B_at_RRR(
    u16 * const bayer,
    u32 row,
    u32 col,
    u32 nrows,
    u32 ncols)
{
    /*
     * [0   0 -1.5 0  0
     *  0   2  0   2  0
     * -1.5 0  6   0 -1.5
     *  0   2  0   2  0
     *  0   0 -1.5 0  0] /8;
     */
    const u16 pos =
        2 * bayer[ncols*(row-1) + (col-1)] +
        2 * bayer[ncols*(row-1) + (col+1)] +
        6 * bayer[ncols*(row) + (col)] +
        2 * bayer[ncols*(row+1) + (col-1)] +
        2 * bayer[ncols*(row+1) + (col+1)];
    const u16 neg =
       (3 * bayer[ncols*(row-2) + (col)] +
        3 * bayer[ncols*(row) + (col-2)] +
        3 * bayer[ncols*(row) + (col+2)] +
        3 * bayer[ncols*(row+2) + (col)]);

    return compute_and_clamp_pixel_fractional_neg(pos, neg);
}
    
void wami_debayer(
    u16 * const bayer,
    rgb_pixel * debayered,
    u32 nrows,
    u32 ncols)
{
    u32 row, col;

    /*
     * Demosaic the following Bayer pattern:
     * R G ...
     * G B ...
     * ... ...
     */

    /* Copy red pixels through directly */
    for (row = PAD; row < nrows - PAD; row += 2)
    {
        for (col = PAD; col < ncols - PAD; col += 2)
        {
            debayered[(ncols-(2*PAD))*(row-PAD) + (col-PAD)].r = bayer[ncols*(row) + (col)];
        }
    }

    /* Copy top-right green pixels through directly */
    for (row = PAD; row < nrows - PAD; row += 2)
    {
        for (col = PAD+1; col < ncols - PAD; col += 2)
        {
            debayered[(ncols-(2*PAD))*(row-PAD) + (col-PAD)].g = bayer[ncols*(row) + (col)];
        }
    }

    /* Copy bottom-left green pixels through directly */
    for (row = PAD+1; row < nrows - PAD; row += 2)
    {
        for (col = PAD; col < ncols - PAD; col += 2)
        {
            debayered[(ncols-(2*PAD))*(row-PAD) + (col-PAD)].g = bayer[ncols*(row) + (col)];
        }
    }

    /* Copy blue pixels through directly */
    for (row = PAD+1; row < nrows - PAD; row += 2)
    {
        for (col = PAD+1; col < ncols - PAD; col += 2)
        {
            debayered[(ncols-(2*PAD))*(row-PAD) + (col-PAD)].b = bayer[ncols*(row) + (col)];
        }
    }

    /* Interpolate green pixels at red pixels */
    for (row = PAD; row < nrows - PAD; row += 2)
    {
        for (col = PAD; col < ncols - PAD; col += 2)
        {
            debayered[(ncols-(2*PAD))*(row-PAD) + (col-PAD)].g = interp_G_at_RRR_or_G_at_BBB(
                bayer, row, col, nrows, ncols);
        }
    }

    /* Interpolate green pixels at blue pixels */
    for (row = PAD+1; row < nrows - PAD; row += 2)
    {
        for (col = PAD+1; col < ncols - PAD; col += 2)
        {
            debayered[(ncols-(2*PAD))*(row-PAD) + (col-PAD)].g = interp_G_at_RRR_or_G_at_BBB(
                bayer, row, col, nrows, ncols);
        }
    }

    /* Interpolate red pixels at green pixels, red row, blue column */
    for (row = PAD; row < nrows - PAD; row += 2)
    {
        for (col = PAD+1; col < ncols - PAD; col += 2)
        {
            debayered[(ncols-(2*PAD))*(row-PAD) + (col-PAD)].r = interp_R_at_GRB_or_B_at_GBR(
                bayer, row, col, nrows, ncols);
        }
    }

    /* Interpolate blue pixels at green pixels, blue row, red column */
    for (row = PAD+1; row < nrows - PAD; row += 2)
    {
        for (col = PAD; col < ncols - PAD; col += 2)
        {
            debayered[(ncols-(2*PAD))*(row-PAD) + (col-PAD)].b = interp_R_at_GRB_or_B_at_GBR(
                bayer, row, col, nrows, ncols);
        }
    }

    /* Interpolate red pixels at green pixels, blue row, red column */
    for (row = PAD+1; row < nrows - PAD; row += 2)
    {
        for (col = PAD; col < ncols - PAD; col += 2)
        {
            debayered[(ncols-(2*PAD))*(row-PAD) + (col-PAD)].r = interp_R_at_GBR_or_B_at_GRB(
                bayer, row, col, nrows, ncols);
        }
    }
 
    /* Interpolate blue pixels at green pixels, red row, blue column */
    for (row = PAD; row < nrows - PAD; row += 2)
    {
        for (col = PAD+1; col < ncols - PAD; col += 2)
        {
            debayered[(ncols-(2*PAD))*(row-PAD) + (col-PAD)].b = interp_R_at_GBR_or_B_at_GRB(
                bayer, row, col, nrows, ncols);
        }
    }

    /* Interpolate red pixels at blue pixels, blue row, blue column */
    for (row = PAD+1; row < nrows - PAD; row += 2)
    {
        for (col = PAD+1; col < ncols - PAD; col += 2)
        {
            debayered[(ncols-(2*PAD))*(row-PAD) + (col-PAD)].r = interp_R_at_BBB_or_B_at_RRR(
                bayer, row, col, nrows, ncols);
        }
    }

    /* Interpolate blue pixels at red pixels, red row, red column */
    for (row = PAD; row < nrows - PAD; row += 2)
    {
        for (col = PAD; col < ncols - PAD; col += 2)
        {
            debayered[(ncols-(2*PAD))*(row-PAD) + (col-PAD)].b = interp_R_at_BBB_or_B_at_RRR(
                bayer, row, col, nrows, ncols);
        }
    }
}
