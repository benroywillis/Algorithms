# cuBLAS binaries were downloaded from https://developer.nvidia.com/cuda-downloads?target_os=Linux&target_arch=x86_64&Distribution=Ubuntu&target_version=22.04&target_type=deb_local
# example was from https://github.com/NVIDIA/CUDALibrarySamples/blob/master/cuBLAS/Level-3/gemm/CMakeLists.txt

include ../../Environment.mk
SOURCE=GEMM
SUFFIX=.cu
POLLY_SHOW=
SIZE?=512
PRECISION?=0 # 0 for float, anything else for double
THREADS_PER_BLOCK?= 256
CHECK?=0
CXXFLAGS += -DSIZE=$(SIZE) -DPRECISION=$(PRECISION) -DTHREADS_PER_BLOCK=$(THREADS_PER_BLOCK) -DCHECK=$(CHECK) -L/usr/local/cuda-12.4/targets/x86_64-linux/lib/
D_LINKS=-lcublas
include ../../BuildFlow.mk
