include ../../../Environment.mk

PERFECT_LIB_PATH=../../../lib/PERFECT/
SOURCE=sar_kernel1_driver
SUFFIX=.c
ADDSOURCE=sar_interp1$(SUFFIX) ../lib/sar_utils$(SUFFIX) $(PERFECT_LIB_PATH)timing/timer$(SUFFIX)
CFLAGS += -DINPUT_SIZE=1
INCLUDE=-I. -I../lib/ -I$(PERFECT_LIB_PATH)
RARGS=../data/
POLLY_SHOW=

include ../../../BuildFlow.mk
