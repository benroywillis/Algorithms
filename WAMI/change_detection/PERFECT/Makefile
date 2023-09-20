include ../../../Environment.mk

PERFECT_LIB_PATH=../../../lib/PERFECT/
SOURCE=change_detection
SUFFIX=.c
ADDSOURCE=wami_gmm$(SUFFIX) wami_morpho$(SUFFIX) ../../lib/wami_utils$(SUFFIX) $(PERFECT_LIB_PATH)timing/timer$(SUFFIX)
CFLAGS += -DINPUT_SIZE=1
INCLUDE=-I$(TRACEATLAS_ROOT)include/ -I. -I$(PERFECT_LIB_PATH) -I../../lib/
POLLY_SHOW=
RARGS=../../../Data/

include ../../../BuildFlow.mk
