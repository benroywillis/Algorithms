include ../../Environment.mk

SOURCE=OptimizedGEMM
D_LINKS=-lm -lomp
NUM_THREADS?=1
SIZE?=512
# 0 for float, 1 for double
PRECISION?=0
CFLAGS+=-mavx -fopenmp -DSIZE=$(SIZE) -DPRECISION=$(PRECISION) -DOMP_NUM_THREADS=$(NUM_THREADS)
INCLUDE=-I$(TRACEATLAS_ROOT)include/
include ../../BuildFlow.mk
