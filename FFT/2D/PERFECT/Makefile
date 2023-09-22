include ../../../Environment.mk

PERFECT_LIB_DIR=../../../lib/PERFECT/
SOURCE=2DFFT
SUFFIX=.c
ADDSOURCE=fft$(SUFFIX) octave$(SUFFIX) $(PERFECT_LIB_DIR)timing/timer$(SUFFIX)

D_LINKS=-lm
DEBUG=-g3
INCLUDE=-I$(TRACEATLAS_ROOT)include/ -I. -I$(PERFECT_LIB_DIR)
LIBRARIES=
POLLY_SHOW=
CFLAGS += -DINPUT_SIZE=1
include ../../../BuildFlow.mk
