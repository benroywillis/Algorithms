include ../../Environment.mk
SOURCE=GEMM
SUFFIX=.cpp
D_LINKS=-lm
INCLUDE=-I$(TRACEATLAS_ROOT)include/Profile/Backend/Utilities/ -I../../TimingLib/
POLLY_SHOW=
SIZE?=64
CXXFLAGS += -DSIZE=$(SIZE)
include ../../BuildFlow.mk
