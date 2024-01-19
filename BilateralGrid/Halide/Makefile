include ../../Environment.mk

HALIDE=1
HALIDE_AUTOSCHEDULE=1
CFLAGS+=-DHALIDE_AUTOSCHEDULE=$(HALIDE_AUTOSCHEDULE)

# In order for this variable to work, your run files need to be named
# $(SOURCE)_generate.cpp $(SOURCE)_run.cpp
# and your generator (-g <generator_name>) needs to match this variable
SOURCE=bilateral_grid
SUFFIX=.cpp

D_LINKS=-ljpeg -lpng -lpthread

SIGMA_S=1.0
SIGMA_R=1.0
INPUT_IMAGE=../../BilateralFilter/Data/john.png
OUTPUT_IMAGE=../../BilateralFilter/Data/filteredjohn.png
RARGS=$(INPUT_IMAGE) $(OUTPUT_IMAGE) $(SIGMA_R) $(SIGMA_S)

POLLY_SHOW=

include ../../BuildFlow.mk
