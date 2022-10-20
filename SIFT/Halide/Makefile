include ../../Environment.mk

DASH_DATA=/mnt/nobackup-09/Dash/Data/
CFLAGS+=-DDASH_DATA=\"$(DASH_DATA)\" -std=c++17

# In order for this variable to work, your run files need to be named
# $(SOURCE)_generate.cpp $(SOURCE)_run.cpp
# and your generator (-g <generator_name>) needs to match this variable
SOURCE=sift
SUFFIX=.cpp

D_LINKS=-ljpeg -lpng -lpthread

AUTOSCHEDULE=0
HALIDE=1
HALIDE_THREADS?=1
HALIDE_AUTOSCHEDULE=0

OCTAVES=4
INTERVALS=5
CUR_THR=1.6
CON_THR=1.4142
INPUT_IMAGE=../../Data/Input/sample_1920_1280.jpg
OUTPUT_IMAGE=../../Data/Output/IIRBlur_Halide_sample_1920_1280.jpg
RARGS=$(OCTAVES) $(INTERVALS) $(CUR_THR) $(CON_THR) $(INPUT_IMAGE) $(OUTPUT_IMAGE) $(HALIDE_THREADS)

POLLY_SHOW=

include ../../BuildFlow.mk
