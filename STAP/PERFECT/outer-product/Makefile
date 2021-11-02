include ../../../Environment.mk

PERFECT_LIB_PATH=../../../lib/PERFECT/
SOURCE=stap_kernel1_driver
SUFFIX=.c
ADDSOURCE=stap_covariance_estimate$(SUFFIX) ../lib/stap_utils$(SUFFIX) $(PERFECT_LIB_PATH)timing/timer.c
CFLAGS += -DINPUT_SIZE=1
RARGS=../data/
INCLUDE=-I$(TRACEATLAS_ROOT)include/ -I../lib/ -I$(PERFECT_LIB_PATH)
POLLY_SHOW=

include ../../../BuildFlow.mk
