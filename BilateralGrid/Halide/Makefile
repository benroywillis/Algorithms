include ../../Environment.mk

DASH_DATA=/mnt/nobackup-09/Dash/Data/
CFLAGS+=-DDASH_DATA=\"$(DASH_DATA)\" -std=c++17

# In order for this variable to work, your run files need to be named
# $(SOURCE)_generate.cpp $(SOURCE)_run.cpp
# and your generator (-g <generator_name>) needs to match this variable
SOURCE=bilateral_grid
SUFFIX=.cpp

D_LINKS=-ljpeg -lpng -lpthread

SIGMA_S=1
SIGMA_R=1.0
INPUT_IMAGE=../../BilateralFilter/Data/john.png
OUTPUT_IMAGE=../../BilateralFilter/Data/filteredjohn.png
RARGS=$(INPUT_IMAGE) $(OUTPUT_IMAGE) $(SIGMA_R) $(SIGMA_S)

AUTOSCHEDULE=0
HALIDE=1

POLLY_SHOW=

include ../../BuildFlow.mk
