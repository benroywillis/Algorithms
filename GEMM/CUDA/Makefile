include ../../Environment.mk
SOURCE=GEMM
SUFFIX=.cu
POLLY_SHOW=
SIZE?=64
PRECISION?=0 # 0 for float, anything else for double
THREADS_PER_BLOCK?= 256
CHECK?=0
CXXFLAGS += -DSIZE=$(SIZE) -DPRECISION=$(PRECISION) -DTHREADS_PER_BLOCK=$(THREADS_PER_BLOCK) -DCHECK=$(CHECK)
include ../../BuildFlow.mk
