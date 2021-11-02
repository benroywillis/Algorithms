include ../../../Environment.mk

PERFECT_LIB_PATH=../../../lib/PERFECT/
SOURCE=stap_app_driver
SUFFIX=.c
ADDSOURCE=corner_turn$(SUFFIX) fft$(SUFFIX) stap_apply_weighting$(SUFFIX) stap_covariance_estimate$(SUFFIX) stap_system_solver$(SUFFIX) stap_utils$(SUFFIX)
CFLAGS += -DINPUT_SIZE=1
RARGS=../data/
INCLUDE=-I$(TRACEATLAS_ROOT)include/ -I../lib/
POLLY_SHOW=

include ../../../BuildFlow.mk
