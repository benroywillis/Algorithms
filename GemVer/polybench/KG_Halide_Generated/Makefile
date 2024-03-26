include ../../../Environment.mk

SIZE?=500
CFLAGS+= -std=c++17 -DSIZE=$(SIZE)
# In order for this variable to work, your run files need to be named
# $(SOURCE)_generate.cpp $(SOURCE)_run.cpp
# and your generator (-g <generator_name>) needs to match this variable
SOURCE=KernelGrammar_gemver
SUFFIX=.cpp

HALIDE=1
HALIDE_THREADS?=1
HALIDE_AUTOSCHEDULE=1
# can also be Mullapudi2016, Li2018, Adams2019, Anderson2021 (for GPU)
HALIDE_AUTOSCHEDULER=Adams2019

RARGS=$(HALIDE_THREADS)

POLLY_SHOW=

include ../../../BuildFlow.mk
