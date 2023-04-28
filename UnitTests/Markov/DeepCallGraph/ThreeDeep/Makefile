include ../../../../Environment.mk
SOURCE=ThreeDeep
RARGS=
OPFLAG=-O0
POLLY_SHOW=

LLVM_INSTALL=/mnt/heorot-10/bwilli46/Installs/LLVM12/release/
TRACEATLAS_INSTALL=/mnt/heorot-10/bwilli46/Installs/TraceAtlas/relwithdebinfo_LLVM12/
CC=$(LLVM_INSTALL)bin/clang

auto_markov_past_test:
	BLOCK_FILE=BlockInfo_ThreeDeep.json $(CC) -fuse-ld=$(LLVM_INSTALL)bin/ld.lld -O1 -lpapi -lz -lpthread -flegacy-pass-manager -Xclang -load -Xclang $(TRACEATLAS_INSTALL)lib/AtlasPasses.so ThreeDeep.c $(TRACEATLAS_INSTALL)lib/libAtlasBackend.so $(LLVM_INSTALL)lib/libLLVMSupport.a -o ThreeDeep.markov.native
	LD_LIBRARY_PATH=$(TRACEATLAS_INSTALL)lib/ MARKOV_FILE=ThreeDeep.bin ./ThreeDeep.markov.native

include ../../../../BuildFlow.mk
