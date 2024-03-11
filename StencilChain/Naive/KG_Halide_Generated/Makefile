include ../../../Environment.mk

CFLAGS+= -std=c++17

# In order for this variable to work, your run files need to be named
# $(SOURCE)_generate.cpp $(SOURCE)_run.cpp
# and your generator (-g <generator_name>) needs to match this variable
SOURCE=KernelGrammar_StencilChain
ADDSOURCE=../../../BilateralFilter/Naive/BilateralFilter.c
SUFFIX=.cpp

INCLUDE=-I../../../BilateralFilter/Naive/
D_LINKS=-ljpeg -lpng -lpthread

HALIDE=1
HALIDE_THREADS?=1
HALIDE_AUTOSCHEDULE=1
# can also be Mullapudi2016, Li2018, Adams2019, Anderson2021 (for GPU)
HALIDE_AUTOSCHEDULER=Adams2019

#INPUT_IMAGE=../../../Data/Input/sample_640_426.jpg
#OUTPUT_IMAGE=../../../Data/Output/StencilChain_Halide_sample_640_426.jpg
INPUT_IMAGE=../../../Data/Input/sample_1920_1280.bmp
OUTPUT_IMAGE=../../../Data/Output/StencilChain_KernelGrammar_Halide_sample_1920_1280.bmp
RARGS=$(INPUT_IMAGE) $(OUTPUT_IMAGE) $(HALIDE_THREADS)

POLLY_SHOW=

include ../../../BuildFlow.mk
