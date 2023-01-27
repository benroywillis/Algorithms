include ../../Environment.mk

SOURCE=Harris
SUFFIX=.cpp
ADDSOURCE=../../BilateralFilter/Naive/BilateralFilter.c
INCLUDE=-I../../BilateralFilter/Naive/

# runtime args
INPUT_IMAGE=../../Data/Input/CheckerBoard.bmp
#INPUT_IMAGE=../../Data/Input/sample_1920_1280.bmp
OUTPUT_IMAGE=../../Data/Output/Harris_Naive_CheckerBoard.bmp
#OUTPUT_IMAGE=../../Data/Output/Harris_Naive_sample_1920_1280.bmp
NUM_THREADS?=1
RARGS=$(INPUT_IMAGE) $(OUTPUT_IMAGE) $(NUM_THREADS)

POLLY_SHOW=
include ../../BuildFlow.mk
