include ../../../Environment.mk

DASH_DATA=/mnt/nobackup-09/Dash/Data/
CFLAGS+=-DDASH_DATA=\"$(DASH_DATA)\" -std=c++17 -DINPUT_SIZE=1

# In order for this variable to work, your run files need to be named
# $(SOURCE)_generate.cpp $(SOURCE)_run.cpp
# and your generator (-g <generator_name>) needs to match this variable
SOURCE=interp2
ADDSOURCE=../../PERFECT/lib/sar_utils.c
SUFFIX=.cpp

INCLUDE=-I../../PERFECT/pfa-interp2/ -I../../PERFECT/lib/
D_LINKS=-ljpeg -lpng -lpthread

INPUT_DIRECTORY=../../PERFECT/data/
OUTPUT_IMAGE=output.png
RARGS=$(INPUT_DIRECTORY) $(OUTPUT_IMAGE)
DEBUG=-g0
OPFLAG=-O3
AUTOSCHEDULE=0
HALIDE=1

POLLY_SHOW=

include ../../../BuildFlow.mk
