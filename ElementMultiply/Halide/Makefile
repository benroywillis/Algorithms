include ../../Environment.mk

# In order for this variable to work, your run files need to be named
# $(SOURCE)_generate.cpp $(SOURCE)_run.cpp
# and your generator (-g <generator_name>) needs to match this variable
SOURCE=ElementMultiply
SUFFIX=.cpp
SIZE?=512
CFLAGS += -DSIZE=$(SIZE)
CXXFLAGS += -std=c++17

AUTOSCHEDULE=0
HALIDE=1
HALIDE_THREADS?=1
HALIDE_AUTOSCHEDULE=0

RARGS=$(HALIDE_THREADS)

include ../../BuildFlow.mk
