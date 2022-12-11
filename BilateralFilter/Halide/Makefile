include ../../Environment.mk

DASH_DATA=/mnt/nobackup-09/Dash/Data/
CFLAGS+=-DDASH_DATA=\"$(DASH_DATA)\" -std=c++17

# In order for this variable to work, your run files need to be named
# $(SOURCE)_generate.cpp $(SOURCE)_run.cpp
# and your generator (-g <generator_name>) needs to match this variable
SOURCE=bilateral_filter
SUFFIX=.cpp

D_LINKS=-ljpeg -lpng -lpthread

SIGMA_S=1.0
SIGMA_R=1.0
INPUT_IMAGE=../../Data/Input/john.png
OUTPUT_IMAGE=../../Data/Output/Halide_BilateralFilter_john.png
RARGS=$(SIGMA_S) $(SIGMA_R) $(INPUT_IMAGE) $(OUTPUT_IMAGE)

HALIDE_AUTOSCHEDULE=0
HALIDE=1

POLLY_SHOW=

include ../../BuildFlow.mk
