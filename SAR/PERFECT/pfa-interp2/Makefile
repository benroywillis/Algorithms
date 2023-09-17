include ../../../Environment.mk

PERFECT_LIB_PATH=../../../lib/PERFECT/
SOURCE=pfa-interp2
SUFFIX=.c
ADDSOURCE=sar_interp2$(SUFFIX) ../lib/sar_utils$(SUFFIX) $(PERFECT_LIB_PATH)timing/timer$(SUFFIX)
CFLAGS += -DINPUT_SIZE=1
INCLUDE=-I. -I../lib/ -I$(PERFECT_LIB_PATH)
RARGS=../../../Data/
POLLY_SHOW=

include ../../../BuildFlow.mk
