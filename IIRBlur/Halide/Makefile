include ../../Environment.mk

DASH_DATA=/mnt/nobackup-09/Dash/Data/
CFLAGS+=-DDASH_DATA=\"$(DASH_DATA)\" -std=c++17

# In order for this variable to work, your run files need to be named
# $(SOURCE)_generate.cpp $(SOURCE)_run.cpp
# and your generator (-g <generator_name>) needs to match this variable
SOURCE=iir_blur
SUFFIX=.cpp

D_LINKS=-ljpeg -lpng -lpthread

INPUT_IMAGE=../../BilateralFilter/Data/john.png
OUTPUT_IMAGE=../../BilateralFilter/Data/filteredjohn.png
RARGS= $(INPUT_IMAGE) $(OUTPUT_IMAGE) $(HALIDE_THREADS)

AUTOSCHEDULE=0
HALIDE=1

POLLY_SHOW=

include ../../BuildFlow.mk
