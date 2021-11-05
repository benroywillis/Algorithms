include ../../../Environment.mk

DASH_DATA=/mnt/nobackup-09/Dash/Data/
CFLAGS+=-DDASH_DATA=\"$(DASH_DATA)\" -std=c++17

# In order for this variable to work, your run files need to be named
# $(SOURCE)_generate.cpp $(SOURCE)_run.cpp
# and your generator (-g <generator_name>) needs to match this variable
SOURCE=LucasKanade
ADDSOURCE=../../../lib/PERFECT/octave/octave.c
INCLUDE=-I../../../lib/PERFECT/
SUFFIX=.cpp

D_LINKS=-ljpeg -lpng -lpthread

N_ROWS=512
N_COLS=512
INPUT_DX=../../PERFECT/data/small_dx.mat
INPUT_DY=../../PERFECT/data/small_dy.mat
OUTPUT_IMAGE=hessian.png
RARGS=$(N_ROWS) $(N_COLS) $(INPUT_DX) $(INPUT_DY) $(OUTPUT_IMAGE)

AUTOSCHEDULE=0
HALIDE=1
DEBUG=-g3
POLLY_SHOW=

include ../../../BuildFlow.mk
