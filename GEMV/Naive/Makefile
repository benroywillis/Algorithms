include ../../Environment.mk
SOURCE=GEMV
SUFFIX=.cpp
SIZE?=512
CFLAGS += -DSIZE=$(SIZE)
D_LINKS=-lm
INCLUDE=-I$(TRACEATLAS_ROOT)include/Profile/Backend/Utilities/
POLLY_SHOW=
include ../../BuildFlow.mk
