LLVM_INSTALL=/mnt/heorot-10/bwilli46/LLVM9/install-release/

CC=$(LLVM_INSTALL)bin/clang
CXX=$(LLVM_INSTALL)bin/clang++
OPT=$(LLVM_INSTALL)bin/opt
LDFLAGS?=-flto -fuse-ld=lld-9 -Wl,--plugin-opt=emit-llvm
#LDFLAGS?=-c -emit-llvm -g3 -O0

SOURCE=GEMM
D_LINKS=-lm
DEBUG=
#TRACEATLAS_ROOT=/home/bwilli46/Install/TraceAtlas/build2/
#TRACEATLAS_ROOT=/home/bwilli46/TraceAtlas/build2/
TRACEATLAS_ROOT=/home/bwilli46/Install/TraceAtlas_dev/
INCLUDE=-I$(TRACEATLAS_ROOT)include/

# polly flags
OPFLAGS=-O0
# turn polly on in compilation pass
POLLYFLAGS=$(OPFLAGS) -mllvm -polly
## breakdown polly transformation steps
# transforms the input program to a canonical form polly can understand
OPTFLAGS1=-S -polly-canonicalize
# print detected scops
OPTFLAGS2.0=-basic-aa -polly-ast -analyze 
OPTFLAGS2.1=-polly-process-unprofitable -polly-use-llvm-names
# Highlight detected scops in the CFG of the program
OPTFLAGS3=-polly-use-llvm-names -basic-aa#-view-scops # -disable-output
OMPFLAGS =-polly-parallel -lgomp

all: Instance_$(SOURCE).json

$(SOURCE).bc : $(SOURCE).c
	$(CC) $(LDFLAGS) $(INCLUDE) $< -o $@

$(SOURCE).markov.bc: $(SOURCE).bc
	$(OPT) -load $(TRACEATLAS_ROOT)/lib/AtlasPasses.so -Markov $< -o $@

$(SOURCE).instance.bc : $(SOURCE).bc
	$(OPT) -load $(TRACEATLAS_ROOT)/lib/AtlasPasses.so -Instance $< -o $@

$(SOURCE).markov.native : $(SOURCE).markov.bc
	$(CXX) -fuse-ld=lld-9 $(D_LINKS) $(TRACEATLAS_ROOT)/lib/libAtlasBackend.a $< -o $@

$(SOURCE).instance.native : $(SOURCE).instance.bc
	$(CXX) -fuse-ld=lld-9 $(D_LINKS) $(TRACEATLAS_ROOT)/lib/libAtlasBackend.a $< -o $@

$(SOURCE).bin : $(SOURCE).markov.native
	BLOCK_FILE=BlockInfo_$(SOURCE).json MARKOV_FILE=$(SOURCE).bin ./$<

kernel_$(SOURCE).json : $(SOURCE).bin
	$(TRACEATLAS_ROOT)/bin/newCartographer -i $< -b $(SOURCE).bc -bi BlockInfo_$(SOURCE).json -d dot_$(SOURCE).dot -o $@

Instance_$(SOURCE).json : $(SOURCE).instance.native kernel_$(SOURCE).json
	KERNEL_FILE=kernel_$(SOURCE).json INSTANCE_FILE=$@ ./$<

# regular tik
tik_$(SOURCE).bc : kernel_$(SOURCE).json $(SOURCE).bc
	$(TRACEATLAS_ROOT)bin/tik -j $^ -o $@

ts_$(SOURCE).bc : tik_$(SOURCE).bc $(SOURCE).bc
	$(TRACEATLAS_ROOT)bin/tikSwap -g -t $< -b $(SOURCE).bc -o $@

ts_$(SOURCE).exec : ts_$(SOURCE).bc tik_$(SOURCE).bc
	$(CXX) $^ -o $@

ts_$(SOURCE)_run : ts_$(SOURCE).exec
	./$<

# tik with polly
tik_polly_$(SOURCE).bc : tik_$(SOURCE).bc
	$(CC) $(LDFLAGS) $(POLLY_FLAGS) $< -o $@

ts_polly_$(SOURCE).bc : tik_polly_$(SOURCE).bc $(SOURCE).bc
	$(TRACEATLAS_ROOT)bin/tikSwap -g -t $< -b $(SOURCE).bc -o $@

ts_polly_$(SOURCE).exec : ts_polly_$(SOURCE).bc tik_polly_$(SOURCE).bc
	$(CXX) $^ -o $@

ts_polly_$(SOURCE)_run : ts_polly_$(SOURCE).exec
	./$<

# just builds the source code into elf form
elf : $(SOURCE).c
	$(CC) $(D_LINKS) -g3 -O0 $(DEBUG) $< -o $(SOURCE).elf

display : elf
	./$(SOURCE).elf

clean:
	rm -rf *.bc *.ll *.tr* *.bin *.json *.exec *.elf *.native *.dot
