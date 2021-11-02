include ../../../Environment.mk

PERFECT_LIB_PATH=../../../lib/PERFECT/
SOURCE=main
SUFFIX=.c
ADDSOURCE=lucas_kanade$(SUFFIX) interpolate$(SUFFIX) ../lib/wami_utils$(SUFFIX) $(PERFECT_LIB_PATH)timing/timer$(SUFFIX) $(PERFECT_LIB_PATH)octave/octave$(SUFFIX)
CFLAGS += -DINPUT_SIZE=1
INCLUDE=-I$(TRACEATLAS_ROOT)include/ -I. -I$(PERFECT_LIB_PATH) -I../lib/
RARGS=../data/
POLLY_SHOW=

include ../../../BuildFlow.mk
