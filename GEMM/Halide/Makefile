include ../../Environment.mk

# In order for this variable to work, your run files need to be named
# $(SOURCE)_generate.cpp $(SOURCE)_run.cpp
# and your generator (-g <generator_name>) needs to match this variable
SOURCE=MatrixMultiply
SUFFIX=.cpp
PRECISION?=double
SIZE?=512
TOLERANCE?=0.001

HALIDE=1
HALIDE_THREADS?=1
HALIDE_AUTOSCHEDULE=1

CFLAGS += -DHALIDE_AUTOSCHEDULE=$(HALIDE_AUTOSCHEDULE) -DPRECISION=$(PRECISION) -DSIZE=$(SIZE) -DTOLERANCE=$(TOLERANCE)
CXXFLAGS += -std=c++17

RARGS=$(SIZE) $(SIZE) $(HALIDE_THREADS)

include ../../BuildFlow.mk
