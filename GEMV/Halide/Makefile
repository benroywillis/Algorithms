include ../../Environment.mk

# In order for this variable to work, your run files need to be named
# $(SOURCE)_generate.cpp $(SOURCE)_run.cpp
# and your generator (-g <generator_name>) needs to match this variable
SOURCE=GEMV
SUFFIX=.cpp
PRECISION?=double
SIZE?=512
CFLAGS += -DPRECISION=$(PRECISION) -DSIZE=$(SIZE)
CXXFLAGS += -std=c++17

AUTOSCHEDULE=0
HALIDE=1
HALIDE_THREADS?=1
HALIDE_AUTOSCHEDULE=0

RARGS=$(SIZE) $(SIZE) $(HALIDE_THREADS)

include ../../BuildFlow.mk
