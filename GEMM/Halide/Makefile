include ../../Environment.mk

# In order for this variable to work, your run files need to be named
# $(SOURCE)_generate.cpp $(SOURCE)_run.cpp
# and your generator (-g <generator_name>) needs to match this variable
SOURCE=GEMM
SUFFIX=.cpp
D_LINKS=-lpthread -lm
PRECISION?=double
SIZE?=512
TOLERANCE?=0.001
CFLAGS += -DPRECISION=$(PRECISION) -DSIZE=$(SIZE) -DTOLERANCE=$(TOLERANCE)

HALIDE=1
include ../../BuildFlow.mk
