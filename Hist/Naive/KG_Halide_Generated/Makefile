include ../../../Environment.mk


SIZE?=512
PRECISION?=0
CHECK?=0
PRINT_TIMES?=0
# In order for this variable to work, your run files need to be named
# $(SOURCE)_generate.cpp $(SOURCE)_run.cpp
# and your generator (-g <generator_name>) needs to match this variable
SOURCE=KernelGrammar_Hist
SUFFIX=.cpp

HALIDE=1
HALIDE_THREADS?=1
HALIDE_AUTOSCHEDULE=0
# can also be Mullapudi2016, Li2018, Adams2019, Anderson2021 (for GPU)
HALIDE_AUTOSCHEDULER=Adams2019

RARGS=$(HALIDE_THREADS)
CFLAGS += -DPRECISION=$(PRECISION) -DSIZE=$(SIZE) -DCHECK=$(CHECK) -DPRINT_TIMES=$(PRINT_TIMES)
POLLY_SHOW=

include ../../../BuildFlow.mk
