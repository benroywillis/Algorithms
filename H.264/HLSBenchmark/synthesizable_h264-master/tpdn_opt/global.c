
#include "global.h"

char intra_pred_mode[PicWidthInMBs*4][FrameHeightInMbs*4];
unsigned char nz_coeff_luma[PicWidthInMBs*4][FrameHeightInMbs*4];
unsigned char nz_coeff_chroma[2][PicWidthInMBs*2][FrameHeightInMbs*2];
unsigned char Mb_prediction_type[PicWidthInMBs][FrameHeightInMbs];

//4. frame information as decoded frame buffer




const unsigned char QPc[52];
const unsigned char SNGL_SCAN[16][2];
const unsigned char FIELD_SCAN[16][2];
const unsigned char decode_block_scan[4][4];


