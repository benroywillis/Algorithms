include ../../Environment.mk

SOURCE=StencilChain
SUFFIX=.cpp
ADDSOURCE=../../BilateralFilter/Naive/BilateralFilter.c

# complile time args
PRECISION?=float
CFLAGS += -DPRECISION=$(PRECISION)

# runtime args
INPUT_IMAGE=../../Data/Input/sample_1920_1280.bmp
OUTPUT_IMAGE=../../Data/Output/StencilChain_Naive_sample_1920_1280.bmp
RARGS=$(INPUT_IMAGE) $(OUTPUT_IMAGE)
D_LINKS=-lm
INCLUDE=-I$(TRACEATLAS_ROOT)include/ -I../../BilateralFilter/Naive/

POLLY_SHOW=-mllvm -polly-show-only
include ../../BuildFlow.mk
