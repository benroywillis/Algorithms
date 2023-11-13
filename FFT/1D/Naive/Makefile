include ../../../Environment.mk

SOURCE=FFT
#INPUT_DIM_N=512
#RARGS=$(INPUT_DIM_N)

D_LINKS=-lm
DEBUG=-g3
INCLUDE=-I$(TRACEATLAS_ROOT)include/
LIBRARIES=
POLLY_SHOW=

SIZE?=4096
CFLAGS += -DSIZE=$(SIZE)

include ../../../BuildFlow.mk
