include ../../../Environment.mk

PERFECT_LIB_PATH=../../../lib/PERFECT/
SOURCE=InnerProduct
SUFFIX=.c
ADDSOURCE=stap_apply_weighting$(SUFFIX) ../../lib/stap_utils$(SUFFIX) $(PERFECT_LIB_PATH)timing/timer.c
CFLAGS += -DINPUT_SIZE=1
RARGS=../../../Data/Input/
INCLUDE=-I$(TRACEATLAS_ROOT)include/ -I../../lib/ -I$(PERFECT_LIB_PATH)
POLLY_SHOW=

include ../../../BuildFlow.mk
