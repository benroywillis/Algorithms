include ../../../Environment.mk

PERFECT_LIB_PATH=../../../lib/PERFECT/
SOURCE=wami_kernel1_driver
SUFFIX=.c
ADDSOURCE=wami_debayer$(SUFFIX) ../lib/wami_utils$(SUFFIX) $(PERFECT_LIB_PATH)timing/timer$(SUFFIX)
CFLAGS += -DINPUT_SIZE=1
INCLUDE=-I$(TRACEATLAS_ROOT)include/ -I. -I$(PERFECT_LIB_PATH) -I../lib/
RARGS=../data/
POLLY_SHOW=
DEBUG=-g0
OPFLAG=-O3

include ../../../BuildFlow.mk
