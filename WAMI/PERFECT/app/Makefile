include ../../../Environment.mk

PERFECT_LIB_PATH=../../../lib/PERFECT/
SOURCE=main
SUFFIX=.c
ADDSOURCE=fileio$(SUFFIX) gradient$(SUFFIX) interpolate$(SUFFIX) lucas_kanade$(SUFFIX) matrix_ops$(SUFFIX) octave_u16$(SUFFIX) rgb_to_grayscale$(SUFFIX) wami_debayer$(SUFFIX) wami_gmm$(SUFFIX) $(PERFECT_LIB_PATH)octave/octave$(SUFFIX) $(PERFECT_LIB_PATH)xmem/xmalloc$(SUFFIX) $(PERFECT_LIB_PATH)timing/timer$(SUFFIX)
CFLAGS += -DINPUT_SIZE=1
INCLUDE=-I$(TRACEATLAS_ROOT)include/ -I. -I$(PERFECT_LIB_PATH) -I../lib/
POLLY_SHOW=

include ../../../BuildFlow.mk
