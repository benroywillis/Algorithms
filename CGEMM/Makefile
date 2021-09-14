
CC?=clang-9
CXX?=clang++-9
OPT?=opt-9
LDFLAGS?=-c -emit-llvm -g3 -O0

SOURCE=CGEMM
D_LINKS=-lm

TRACEATLAS_ROOT=/home/bwilli46/Install/TraceAtlas/build2/
#TRACEATLAS_ROOT=/home/bwilli46/TraceAtlas/build2/

all: kernel_$(SOURCE).json

$(SOURCE).bc : $(SOURCE).c
	$(CC) $(LDFLAGS) $< -o $@

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

Instance_$(SOURCE).json : kernel_$(SOURCE).json $(SOURCE).instance.native
	KERNEL_FILE=$< INSTANCE_FILE=$@ ./$(SOURCE).instance.native

# just builds the source code into elf form
elf : $(SOURCE).c
	$(CC) $(D_LINKS) -g3 -O0 $< -o $(SOURCE).elf

clean:
	rm -rf *.bc *.ll *.tr* *.bin *.json *.exec *.elf *.native *.dot
