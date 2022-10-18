include ../../Environment.mk

SOURCE=SIFT
SUFFIX=.cpp
ADDSOURCE=../../BilateralFilter/Naive/BilateralFilter.c

# complile time args
PRECISION?=float
CFLAGS += -DPRECISION=$(PRECISION)

# runtime args
OCTAVES?=4
INTERVALS?=5
CUR_THR?=1.6 # optimal values as presented in the D.Lowe publication "Distinctive Image Featurs from Scale-Invariant Keypoints"
CON_THR?=1.4142 # root(2)
INPUT_IMAGE=../../Data/Input/sample_1920_1280.bmp
OUTPUT_IMAGE=../../Data/Output/SIFT_Naive_sample_1920_1280.bmp
RARGS=$(OCTAVES) $(INTERVALS) $(CUR_THR) $(CON_THR) $(INPUT_IMAGE) $(OUTPUT_IMAGE)
D_LINKS=-lm
INCLUDE=-I$(TRACEATLAS_ROOT)include/ -I../../BilateralFilter/Naive/

POLLY_SHOW=
include ../../BuildFlow.mk
