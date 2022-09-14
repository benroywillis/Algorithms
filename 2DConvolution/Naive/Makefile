include ../../Environment.mk

SOURCE=2DConv
SUFFIX=.cpp
ADDSOURCE=../../BilateralFilter/Naive/BilateralFilter.c
CFLAGS += -DPRECISION=float
INPUT_IMAGE=../../Data/Input/john.bmp
OUTPUT_IMAGE=../../Data/Output/Naive2DConv_john.bmp
RARGS=$(INPUT_IMAGE) $(OUTPUT_IMAGE)
D_LINKS=-lm
INCLUDE=-I$(TRACEATLAS_ROOT)include/ -I../../BilateralFilter/Naive/

POLLY_SHOW=
include ../../BuildFlow.mk
