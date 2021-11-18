include ../../Environment.mk

DASH_DATA=/mnt/nobackup-09/Dash/Data/
CFLAGS+=-DDASH_DATA=\"$(DASH_DATA)\" #-std=c++17

# In order for this variable to work, your run files need to be named
# $(SOURCE)_generate.cpp $(SOURCE)_run.cpp
# and your generator (-g <generator_name>) needs to match this variable
SOURCE=TwoDConv
ADDSOURCE=../../lib/PERFECT/octave/octave.c
INCLUDE=-I../../lib/PERFECT/
SUFFIX=.cpp

D_LINKS=-ljpeg -lpng -lpthread

INPUT_IMAGE=../PERFECT/input_small.m
OUTPUT_IMAGE=output.png
RARGS=$(INPUT_IMAGE) $(OUTPUT_IMAGE)

AUTOSCHEDULE=0
HALIDE=1

POLLY_SHOW=

include ../../BuildFlow.mk
