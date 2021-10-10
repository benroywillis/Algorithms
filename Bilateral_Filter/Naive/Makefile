LLVM_INSTALL=/mnt/heorot-10/bwilli46/LLVM9/install-release/
#LLVM_INSTALL=/mnt/heorot-10/bwilli46/LLVM12/install-release/
GCC=gcc
GXX=g++
GLD=ld

CC=$(LLVM_INSTALL)bin/clang
CXX=$(LLVM_INSTALL)bin/clang++
OPT=$(LLVM_INSTALL)bin/opt
LDFLAGS?=-flto -fuse-ld=$(LLVM_INSTALL)/bin/ld.lld -Wl,--plugin-opt=emit-llvm
#LDFLAGS?=-c -emit-llvm -g3 -O0

SOURCE=BilateralFilter
SIGMA_S=1.0
SIGMA_R=1.0
INPUT_IMAGE=john.bmp
OUTPUT_IMAGE=filtered_john.bmp

D_LINKS=-lm
DEBUG=-g3
#TRACEATLAS_ROOT=/home/bwilli46/Install/TraceAtlas/build2/
#TRACEATLAS_ROOT=/home/bwilli46/TraceAtlas/build2/
TRACEATLAS_ROOT=/home/bwilli46/Install/TraceAtlas_dev/
INCLUDE=-I$(TRACEATLAS_ROOT)include/

# polly flags
# possibly helpful link: https://groups.google.com/g/polly-dev/c/k5s4dRiZ8rc?pli=1
OPFLAG=-O1
# turn polly on in compilation pass
POLLYFLAGS=$(OPFLAG) -mllvm -polly -mllvm -polly-allow-nonaffine
## breakdown polly transformation steps
# transforms the input program to a canonical form polly can understand
POLLY_OPTFLAGS1=-S -polly-canonicalize
# print detected scops
POLLY_OPTFLAGS2.0=-polly-use-llvm-names -polly-allow-nonaffine-loops -polly-allow-nonaffine-branches -basicaa -polly-scops -analyze
POLLY_OPTFLAGS2.1=-polly-process-unprofitable 
# Highlight detected scops in the CFG of the program
POLLY_OPTFLAGS3=-polly-use-llvm-names -basicaa#-view-scops # -disable-output
OMPFLAGS =-polly-parallel -lgomp

all: Instance_$(SOURCE).json

$(SOURCE).bc : $(SOURCE).c
	$(CC) $(LDFLAGS) $(INCLUDE) $(OPFLAG) $< -o $@

$(SOURCE).markov.bc: $(SOURCE).bc
	$(OPT) -load $(TRACEATLAS_ROOT)/lib/AtlasPasses.so -Markov $< -o $@

$(SOURCE).instance.bc : $(SOURCE).bc
	$(OPT) -load $(TRACEATLAS_ROOT)/lib/AtlasPasses.so -Instance $< -o $@

$(SOURCE).markov.native : $(SOURCE).markov.bc
	$(CXX) -fuse-ld=$(LLVM_INSTALL)bin/ld.lld $(D_LINKS) $(TRACEATLAS_ROOT)/lib/libAtlasBackend.a $< -o $@

$(SOURCE).instance.native : $(SOURCE).instance.bc
	$(CXX) -fuse-ld=$(LLVM_INSTALL)bin/ld.lld $(D_LINKS) $(TRACEATLAS_ROOT)/lib/libAtlasBackend.a $< -o $@

$(SOURCE).bin : $(SOURCE).markov.native
	BLOCK_FILE=BlockInfo_$(SOURCE).json MARKOV_FILE=$(SOURCE).bin ./$< $(SIGMA_S) $(SIGMA_R) $(INPUT_IMAGE) $(OUTPUT_IMAGE)

kernel_$(SOURCE).json : $(SOURCE).bin
	$(TRACEATLAS_ROOT)/bin/newCartographer -i $< -b $(SOURCE).bc -bi BlockInfo_$(SOURCE).json -d dot_$(SOURCE).dot -o $@

Instance_$(SOURCE).json : $(SOURCE).instance.native kernel_$(SOURCE).json
	KERNEL_FILE=kernel_$(SOURCE).json INSTANCE_FILE=$@ ./$< $(SIGMA_S) $(SIGMA_R) $(INPUT_IMAGE) $(OUTPUT_IMAGE)

# regular tik
tik_$(SOURCE).bc : kernel_$(SOURCE).json $(SOURCE).bc
	$(TRACEATLAS_ROOT)bin/tik -S -j $^ -o $@

ts_$(SOURCE).bc : tik_$(SOURCE).bc $(SOURCE).bc
	$(TRACEATLAS_ROOT)bin/tikSwap -S -t $< -b $(SOURCE).bc -o $@

ts_$(SOURCE).exec : ts_$(SOURCE).bc tik_$(SOURCE).bc
	$(CXX) $(OPFLAG) $^ -o $@

ts_$(SOURCE)_run : ts_$(SOURCE).exec
	./$< $(SIGMA_S) $(SIGMA_R) $(INPUT_IMAGE) $(OUTPUT_IMAGE)

# tik with polly
tik_polly_$(SOURCE).bc : tik_$(SOURCE).bc
	$(CC) $(LDFLAGS) $(OPFLAG) $(POLLYFLAGS) -S $< -o $@

ts_polly_$(SOURCE).bc : tik_polly_$(SOURCE).bc $(SOURCE).bc
	$(TRACEATLAS_ROOT)bin/tikSwap -S -t $< -b $(SOURCE).bc -o $@

ts_polly_$(SOURCE).exec : ts_polly_$(SOURCE).bc tik_polly_$(SOURCE).bc
	$(CXX) $(OPFLAG) $^ -o $@

ts_polly_$(SOURCE)_run : ts_polly_$(SOURCE).exec
	./$< $(SIGMA_S) $(SIGMA_R) $(INPUT_IMAGE) $(OUTPUT_IMAGE)

# break polly down into steps
tik_polly_canon.bc : tik_$(SOURCE).bc
	$(OPT) -S $(POLLY_OPTFLAGS_1) $< -o $@

tik_polly_scops : tik_polly_canon.bc
	$(OPT) $(POLLY_OPTFLAGS2.0) $< $(POLLY_OPTFLAGS2.1)

# just polly
$(SOURCE).canonical.bc : $(SOURCE).bc
	$(OPT) -S $(POLLY_OPTFLAGS_1) $< -o $@

$(SOURCE)_polly_scops : $(SOURCE).canonical.bc
	$(OPT) $(POLLY_OPTFLAGS2.0) $< $(POLLY_OPTFLAGS2.1)

# just builds the source code into elf form
elf : $(SOURCE).c
	$(CC) $(INCLUDE) $(D_LINKS) $(OPFLAG) $(DEBUG) $< -o $(SOURCE).elf

run : elf
	./$(SOURCE).elf $(SIGMA_S) $(SIGMA_R) $(INPUT_IMAGE) $(OUTPUT_IMAGE)

gprof_$(SOURCE).elf : $(SOURCE).c
	$(GCC) $(INCLUDE) $(OPFLAG) $(DEBUG) -c -pg -Wno-unused-result $< -o gprof_$(SOURCE).obj
	$(GCC) -pg gprof_$(SOURCE).obj $(D_LINKS) -o $@
	./$@ $(SIGMA_S) $(SIGMA_R) $(INPUT_IMAGE) $(OUTPUT_IMAGE) 

gprof : gprof_$(SOURCE).elf
	gprof -l ./$<

gcov_$(SOURCE).elf : $(SOURCE).c
	$(GXX) $(INCLUDE) $(OPFLAG) $(DEBUG) $(D_LINKS) -Wno-unused-result -fprofile-arcs -ftest-coverage -fPIC $< -o $@

gcov : gcov_$(SOURCE).elf
	./$< $(SIGMA_S) $(SIGMA_R) $(INPUT_IMAGE) $(OUTPUT_IMAGE)
	gcov --all-blocks --branch-probabilities --branch-counts --display-progress --function-summaries $(SOURCE)

perf : elf
	sudo perf stat -d ./BilateralFilter.elf $(SIGMA_S) $(SIGMA_R) $(INPUT_IMAGE) $(OUTPUT_IMAGE)

# can't figure out where oprofile puts the program stdout, so I have to use time -p for now
operf : elf
	sudo time -p operf ./$(SOURCE).elf $(SIGMA_S) $(SIGMA_R) $(INPUT_IMAGE) $(OUTPUT_IMAGE)
	opreport --exclude-dependent --demangle=smart --symbols --threshold=1 > opreport.out

elf_polly : $(SOURCE).c
	$(CC) $(INCLUDE) $(D_LINKS) -mllvm -polly $(OPFLAG) -mllvm -polly-show-only -mllvm -polly-allow-nonaffine -mllvm -polly-allow-nonaffine-branches -mllvm -polly-allow-nonaffine-loops $< -o $(SOURCE).elf_polly

display : elf
	./$(SOURCE).elf

.PHONY:

clean:
	rm -rf *.bc *.ll *.tr* *.bin *.json *.exec *.elf* *.native *.dot *.bc.h *.elf *.obj *.gcda *.gcno *.gcov *.log *.data *.out

clean_oprofile:
	sudo rm -rf oprofile_data
