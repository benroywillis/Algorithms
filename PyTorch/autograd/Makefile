include ../../Environment.mk
SOURCE=autograd
SUFFIX=.cpp
INCLUDE=-I$(TORCH_ROOT)include/ -I$(TORCH_ROOT)include/torch/csrc/api/include/
# for some reason, pytorch archives need to have special flags (or else their ctors and dtors are undefined
CXXFLAGS += -Wl,--gc-sections -Wl,--whole-archive
LIBRARIES=$(TORCH_ROOT)lib/libtorch.a \
$(TORCH_ROOT)lib/libtorch_cpu.a
# then, those flags used for pytorch archives cannot be used for the others, so we turn them off
ARCHIVE_FLAGS=-Wl,--no-whole-archive
LIBRARIES2=$(TORCH_ROOT)lib/libc10.a \
$(TORCH_ROOT)lib/libCaffe2_perfkernels_avx.a \
$(TORCH_ROOT)lib/libCaffe2_perfkernels_avx2.a \
$(TORCH_ROOT)lib/libCaffe2_perfkernels_avx512.a \
$(TORCH_ROOT)lib/libcaffe2_protos.a \
$(TORCH_ROOT)lib/libclog.a \
$(TORCH_ROOT)lib/libcpuinfo.a \
$(TORCH_ROOT)lib/libittnotify.a \
$(TORCH_ROOT)lib/libkineto.a \
$(TORCH_ROOT)lib/libnnpack.a \
$(TORCH_ROOT)lib/libpthreadpool.a \
$(TORCH_ROOT)lib/libprotobuf.a \
$(TORCH_ROOT)lib/libpytorch_qnnpack.a \
$(TORCH_ROOT)lib/libqnnpack.a \
$(TORCH_ROOT)lib/libsleef.a \
$(TORCH_ROOT)lib/libonnx.a \
$(TORCH_ROOT)lib/libonnx_proto.a \
$(TORCH_ROOT)lib/libXNNPACK.a

# the debug version of this archive has an extra d at the end
#$(TORCH_ROOT)lib/libprotobufd.a \
#$(TORCH_ROOT)lib/libasmjit.a \
#$(TORCH_ROOT)lib/libdnnl.a \
#$(TORCH_ROOT)lib/libfbgemm.a \
#$(TORCH_ROOT)lib/libgloo.a \
#$(TORCH_ROOT)lib/libtensorpipe.a \
#$(TORCH_ROOT)lib/libtensorpipe_uv.a \

# nnpack is a mix of elf objects and bitcode (which suggests it has non-HLL stuff in it)
D_LINKS=-lomp $(TORCH_ROOT)lib/libnnpack.a /usr/lib/x86_64-linux-gnu/libnuma.a
include ../../BuildFlow.mk
