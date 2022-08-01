include ../../Environment.mk

SOURCE=PermutoHedralLattice
ADDSOURCE=../../BilateralFilter/Naive/BilateralFilter.c
SUFFIX=.cpp

# runtime args
SIGMA_S=1.0
SIGMA_R=1.0
INPUT_IMAGE=../../BilateralFilter/Data/john.bmp
OUTPUT_IMAGE=../../BilateralFilter/Data/filtered_john.bmp
RARGS=$(SIGMA_S) $(SIGMA_R) $(INPUT_IMAGE) $(OUTPUT_IMAGE)
D_LINKS=-lm
INCLUDE=-I$(TRACEATLAS_ROOT)include/ -I../../BilateralFilter/Naive/ -I../../TimingLib.h

POLLY_SHOW=
include ../../BuildFlow.mk
