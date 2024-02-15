include ../../Environment.mk

SOURCE=IIRBlur
SUFFIX=.cpp
ADDSOURCE=../../BilateralFilter/Naive/BilateralFilter.c

# complile time args
PRECISION?=float
CFLAGS += -DPRECISION=$(PRECISION)

# runtime args
ALPHA=0.5
#INPUT_IMAGE=../../Data/Input/sample_1920_1280.bmp
INPUT_IMAGE=../../Data/Input/sample_640_426.bmp
#OUTPUT_IMAGE=../../Data/Output/IIRBlur_Naive_sample_1920_1280.bmp
OUTPUT_IMAGE=../../Data/Output/IIRBlur_Naive_sample_640_426.bmp
RARGS=$(ALPHA) $(INPUT_IMAGE) $(OUTPUT_IMAGE)
D_LINKS=-lm
INCLUDE=-I$(TRACEATLAS_ROOT)include/ -I../../BilateralFilter/Naive/

POLLY_SHOW=
include ../../BuildFlow.mk
