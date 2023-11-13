include ../../Environment.mk

# In order for this variable to work, your run files need to be named
# $(SOURCE)_generate.cpp $(SOURCE)_run.cpp
# and your generator (-g <generator_name>) needs to match this variable
SOURCE=ComplexMatrixMultiply
SUFFIX=.cpp
# 0 for float 1 for double
PRECISION?=0
SIZE?=512
CFLAGS += -DPRECISION=$(PRECISION) -DSIZE=$(SIZE)
CXXFLAGS += -std=c++17
INCLUDE=-I../../lib/

AUTOSCHEDULE=0
HALIDE=1
HALIDE_THREADS?=1
HALIDE_AUTOSCHEDULE=0

RARGS=$(HALIDE_THREADS)

include ../../BuildFlow.mk
