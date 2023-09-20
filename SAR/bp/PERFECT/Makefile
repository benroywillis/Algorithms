include ../../../Environment.mk

PERFECT_LIB_PATH=../../../lib/PERFECT/
SOURCE=BP
SUFFIX=.c
ADDSOURCE=sar_backprojection$(SUFFIX) ../../lib/sar_utils$(SUFFIX) $(PERFECT_LIB_PATH)timing/timer$(SUFFIX)
CFLAGS += -DINPUT_SIZE=1 -DENABLE_CORRECTNESS_CHECKING
INCLUDE=-I. -I../../lib/ -I../../../lib/PERFECT/
RARGS=../../../Data/
POLLY_SHOW=
DEBUG=-g0
OPFLAG=-O3

include ../../../BuildFlow.mk
