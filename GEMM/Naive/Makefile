include ../../Environment.mk
SOURCE=GEMM
SUFFIX=.c
D_LINKS=-lm
INCLUDE=-I$(TRACEATLAS_ROOT)include/Profile/Backend/Utilities/
POLLY_SHOW=
SIZE?=64
CXXFLAGS += -DSIZE=$(SIZE)
include ../../BuildFlow.mk
