include ../../Environment.mk

DASH_DATA=/mnt/nobackup-09/Dash/Data/
CFLAGS+=-DDASH_DATA=\"$(DASH_DATA)\" -std=c++17 -DINPUT_SIZE=1 -DENABLE_CORRECTNESS_CHECKING

# In order for this variable to work, your run files need to be named
# $(SOURCE)_generate.cpp $(SOURCE)_run.cpp
# and your generator (-g <generator_name>) needs to match this variable
SOURCE=inner_product
SUFFIX=.cpp

D_LINKS=-ljpeg -lpng -lpthread

RARGS=$(INPUT_DIRECTORY) $(OUTPUT_IMAGE)
DEBUG=-g0
OPFLAG=-O3
HALIDE=1
AUTOSCHEDULE=0
HALIDE_AUTOSCHEDULE=0
HALIDE_THREADS?=1

POLLY_SHOW=

include ../../BuildFlow.mk
