include ../../Environment.mk

SOURCE=SIFT
SUFFIX=.cpp
ADDSOURCE=../../BilateralFilter/Naive/BilateralFilter.c

# compile time args
PRECISION?=float
CFLAGS += -DPRECISION=$(PRECISION) -fopenmp 
D_LINKS=-lm -lgomp -lpthread -fopenmp
D_LINKS_PATH=$(LLVM_ROOT)lib/
INCLUDE=-I$(TRACEATLAS_ROOT)include/ -I../../BilateralFilter/Naive/

# runtime args
OCTAVES?=4
INTERVALS?=5
CUR_THR?=10 
CON_THR?=0.09
INPUT_IMAGE=../../Data/Input/sample_1920_1280.bmp
OUTPUT_IMAGE=../../Data/Output/SIFT_Naive_sample_1920_1280.bmp
NUM_THREADS?=1
BIN_VARS=OMP_NUM_THREADS=$(NUM_THREADS)
RARGS=$(OCTAVES) $(INTERVALS) $(CUR_THR) $(CON_THR) $(INPUT_IMAGE) $(OUTPUT_IMAGE)

POLLY_SHOW=
include ../../BuildFlow.mk
