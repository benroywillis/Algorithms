include ../../../Environment.mk

DASH_DATA=/mnt/nobackup-09/Dash/Data/
CFLAGS+=-DDASH_DATA=\"$(DASH_DATA)\" -std=c++17 -DINPUT_SIZE=1

# In order for this variable to work, your run files need to be named
# $(SOURCE)_generate.cpp $(SOURCE)_run.cpp
# and your generator (-g <generator_name>) needs to match this variable
SOURCE=GMM
ADDSOURCE=../../PERFECT/lib/wami_utils.c wami_read.c
INCLUDE=-I../../../lib/PERFECT/ -I../../PERFECT/lib/ -I ../../PERFECT/change_detection/
SUFFIX=.cpp

D_LINKS=-ljpeg -lpng -lpthread

DATA_FOLDER=../../PERFECT/data/
DATA_FILE=small_kernel3_input.bin
OUTPUT_IMAGE=foreground.png
RARGS=$(DATA_FOLDER) $(DATA_FILE) $(OUTPUT_IMAGE)

AUTOSCHEDULE=0
HALIDE=1
DEBUG=-g3
POLLY_SHOW=

include ../../../BuildFlow.mk
