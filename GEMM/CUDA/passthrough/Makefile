include ../../../Environment.mk
SOURCE=passthrough
SUFFIX=.cu
POLLY_SHOW=
SIZE?=64
PRECISION?=0 # 0 for float, anything else for double
CHECK?=1
CXXFLAGS += --gpu-code=sm_86 --gpu-architecture=compute_86 -DSIZE=$(SIZE) -DPRECISION=$(PRECISION) -DCHECK=$(CHECK)
include ../../../BuildFlow.mk
