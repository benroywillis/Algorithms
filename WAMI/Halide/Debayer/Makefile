include ../../../Environment.mk

DASH_DATA=/mnt/nobackup-09/Dash/Data/
CFLAGS+=-DDASH_DATA=\"$(DASH_DATA)\" -std=c++17 -DINPUT_SIZE=1

# In order for this variable to work, your run files need to be named
# $(SOURCE)_generate.cpp $(SOURCE)_run.cpp
# and your generator (-g <generator_name>) needs to match this variable
SOURCE=Debayer
ADDSOURCE=../../PERFECT/lib/wami_utils.c
INCLUDE=-I../../../lib/PERFECT/ -I../../PERFECT/lib/
SUFFIX=.cpp

D_LINKS=-ljpeg -lpng -lpthread

DATA_PATH=../../PERFECT/data/
DATA_FILE=small_kernel1_input.bin
OUTPUT_FILE=output.bin
RARGS=$(DATA_PATH) $(DATA_FILE) $(OUTPUT_FILE)

AUTOSCHEDULE=0
HALIDE=1
DEBUG=-g3
POLLY_SHOW=

include ../../../BuildFlow.mk
