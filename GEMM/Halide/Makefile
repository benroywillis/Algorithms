include ../../Environment.mk

# In order for this variable to work, your run files need to be named
# $(SOURCE)_generate.cpp $(SOURCE)_run.cpp
# and your generator (-g <generator_name>) needs to match this variable
SOURCE=GEMM
SUFFIX=.cpp
D_LINKS=-lpthread
SIGMA_S=1.0
SIGMA_R=1.0
INPUT_IMAGE=john.png
OUTPUT_IMAGE=filteredjohn.png
RARGS=$(SIGMA_S) $(SIGMA_R) $(INPUT_IMAGE) $(OUTPUT_IMAGE)

HALIDE=1
include ../../BuildFlow.mk
