include ../../../Environment.mk

PERFECT_LIB_PATH=../../../lib/PERFECT/
SOURCE=system_solve
SUFFIX=.c
ADDSOURCE=stap_system_solver$(SUFFIX) ../lib/stap_utils$(SUFFIX) $(PERFECT_LIB_PATH)timing/timer.c
CFLAGS += -DINPUT_SIZE=1
RARGS=../../../Data/
INCLUDE=-I$(TRACEATLAS_ROOT)include/ -I../lib/ -I$(PERFECT_LIB_PATH)
POLLY_SHOW=

include ../../../BuildFlow.mk
