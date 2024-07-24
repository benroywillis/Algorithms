include ../../Environment.mk
SOURCE=GEMM
SUFFIX=.cu
POLLY_SHOW=
# input sizes (MxN = MxK * KxN)
M?=512
K?=512
N?=512
# blocking sizes
BM?=64
BK?=8
BN?=64
# tiling sizes
TM?=8
TN?=8
PRECISION?=0 # 0 for float, anything else for double
CHECK?=0
CXXFLAGS += --gpu-code=sm_86 --gpu-architecture=compute_86 -DM=$(M) -DK=$(K) -DN=$(N) -DBM=$(BM) -DBK=$(BK) -DBN=$(BN) -DTM=$(TM) -DTN=$(TN) -DPRECISION=$(PRECISION) -DCHECK=$(CHECK)
include ../../BuildFlow.mk
