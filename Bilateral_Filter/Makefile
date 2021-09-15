
CC?=clang-9
CXX?=clang++-9
OPT?=opt-9
LDFLAGS?=-c -emit-llvm -g3 -O0

SOURCE=BilateralFilter
D_LINKS=-lm
DEBUG=
TRACEATLAS_ROOT=/home/bwilli46/Install/TraceAtlas/build2/
#TRACEATLAS_ROOT=/home/bwilli46/TraceAtlas/build2/
INCLUDE=
SIGMA_S=0.03
SIGMA_R=0.1

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
	BLOCK_FILE=BlockInfo_$(SOURCE).json MARKOV_FILE=$(SOURCE).bin ./$< output.image john.ppm $(SIGMA_S) $(SIGMA_R)

kernel_$(SOURCE).json : $(SOURCE).bin
	$(TRACEATLAS_ROOT)/bin/newCartographer -i $< -b $(SOURCE).bc -bi BlockInfo_$(SOURCE).json -d dot_$(SOURCE).dot -o $@

Instance_$(SOURCE).json : $(SOURCE).instance.native kernel_$(SOURCE).json
	KERNEL_FILE=kernel_$(SOURCE).json INSTANCE_FILE=$@ ./$< output.image john.ppm $(SIGMA_S) $(SIGMA_R)

# just builds the source code into elf form
elf : $(SOURCE).c
	$(CC) $(D_LINKS) -g3 -O0 $(DEBUG) $< -o $(SOURCE).elf

clean:
	rm -rf *.bc *.ll *.tr* *.bin *.json *.exec *.elf *.native *.dot
