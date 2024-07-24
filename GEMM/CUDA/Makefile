include ../../Environment.mk
SOURCE=GEMM
SUFFIX=.cu
POLLY_SHOW=
SIZE?=64
BLOCKSIZE?=32
PRECISION?=0 # 0 for float, anything else for double
THREADS_PER_BLOCK?=32
CHECK?=0
CXXFLAGS += --gpu-code=sm_86 --gpu-architecture=compute_86 -DSIZE=$(SIZE) -DPRECISION=$(PRECISION) -DBLOCKSIZE=$(BLOCKSIZE) -DTHREADS_PER_BLOCK=$(THREADS_PER_BLOCK) -DCHECK=$(CHECK)
include ../../BuildFlow.mk
