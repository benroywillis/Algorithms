include ../../Environment.mk

DASH_DATA=/mnt/nobackup-09/Dash/Data/
CFLAGS+=-DDASH_DATA=\"$(DASH_DATA)\" -std=c++17

# In order for this variable to work, your run files need to be named
# $(SOURCE)_generate.cpp $(SOURCE)_run.cpp
# and your generator (-g <generator_name>) needs to match this variable
SOURCE=HistEq
ADDSOURCE=../../lib/PERFECT/octave/octave.c
INCLUDE=-I../../lib/PERFECT/
SUFFIX=.cpp

D_LINKS=-ljpeg -lpng -lpthread

N_ROWS=480
N_COLS=640
INPUT_IMAGE=../PERFECT/input_small.m
OUTPUT_IMAGE=equalized.png
RARGS=$(N_ROWS) $(N_COLS) $(INPUT_IMAGE) $(OUTPUT_IMAGE)

AUTOSCHEDULE=0
HALIDE=1

POLLY_SHOW=

include ../../BuildFlow.mk
