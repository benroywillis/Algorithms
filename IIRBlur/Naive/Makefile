include ../../Environment.mk

SOURCE=IIRBlur
SUFFIX=.cpp
ADDSOURCE=../../BilateralFilter/Naive/BilateralFilter.c

# complile time args
PRECISION?=float
CFLAGS += -DPRECISION=$(PRECISION)

# runtime args
ALPHA=0.5
INPUT_IMAGE=../../BilateralFilter/Data/john.bmp
OUTPUT_IMAGE=../../BilateralFilter/Data/filtered_john.bmp
RARGS=$(ALPHA) $(INPUT_IMAGE) $(OUTPUT_IMAGE)
D_LINKS=-lm
INCLUDE=-I$(TRACEATLAS_ROOT)include/ -I../../BilateralFilter/Naive/

POLLY_SHOW=
include ../../BuildFlow.mk
