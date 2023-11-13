include ../../Environment.mk
SOURCE=GEMM
SUFFIX=.cpp
D_LINKS=-lm -lomp
# 0 for float, 1 for double, 2 for int, 3 for long
PRECISION?=0
SIZE?=512
CXXFLAGS += -fopenmp -DSIZE=$(SIZE) -DPRECISION=$(PRECISION)
INCLUDE=-I$(TRACEATLAS_ROOT)include/Profile/Backend/Utilities/
POLLY_SHOW=
include ../../BuildFlow.mk
