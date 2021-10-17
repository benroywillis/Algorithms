
GCC=gcc
GXX=g++
GLD=ld

CC=$(LLVM_INSTALL)bin/clang
CXX=$(LLVM_INSTALL)bin/clang++
OPT=$(LLVM_INSTALL)bin/opt
LLD=-fuse-ld=$(LLVM_INSTALL)bin/ld.lld
LDFLAGS?=-flto $(LLD) -Wl,--plugin-opt=emit-llvm
#LDFLAGS?=-c -emit-llvm -g3 -O0

SOURCE?=test
SUFFIX?=.c
D_LINKS?=-lm
DEBUG?=-g3
CFLAGS?=
INCLUDE?=
LIBRARIES?=
RARGS?=
ifeq ($(SUFFIX),.c)
	C=$(CC)
	GC=$(GCC)
else
	C=$(CXX)
	GC=$(GXX)
endif

# polly flags
# possibly helpful link: https://groups.google.com/g/polly-dev/c/k5s4dRiZ8rc?pli=1
OPFLAG=-O1
# turn polly on in compilation pass
POLLYFLAGS=$(OPFLAG) -mllvm -polly -mllvm -polly-allow-nonaffine
# have polly output a bunch of dots that it then attempts to open with libreoffice
POLLY_SHOW?=-mllvm -polly-show-only
# set this to blank if you don't want plly to consider non-affine structures
POLLY_NONAFFINE=-mllvm -polly-allow-nonaffine -mllvm -polly-allow-nonaffine-branches -mllvm -polly-allow-nonaffine-loops
## breakdown polly transformation steps
# transforms the input program to a canonical form polly can understand
POLLY_OPTFLAGS1=-S -polly-canonicalize
# print detected scops
POLLY_OPTFLAGS2.0=-polly-use-llvm-names -polly-allow-nonaffine-loops -polly-allow-nonaffine-branches -basicaa -polly-scops -analyze
POLLY_OPTFLAGS2.1=-polly-process-unprofitable 
# Highlight detected scops in the CFG of the program
POLLY_OPTFLAGS3=-polly-use-llvm-names -basicaa#-view-scops # -disable-output
OMPFLAGS =-polly-parallel -lgomp

all: lastwriter_$(SOURCE).dot

$(SOURCE).bc : $(SOURCE)$(SUFFIX)
	$(C) $(OPFLAG) $(DEBUG) $(LDFLAGS) $(INCLUDE) $(CFLAGS) $(LIBRARIES) $< -o $@

$(SOURCE).markov.bc: $(SOURCE).bc
	$(OPT) -load $(TRACEATLAS_ROOT)/lib/AtlasPasses.so -Markov $< -o $@

$(SOURCE).hotcode.bc: $(SOURCE).bc
	$(OPT) -load $(TRACEATLAS_HC_ROOT)/lib/AtlasPasses.so -Markov $< -o $@

$(SOURCE).instance.bc : $(SOURCE).bc
	$(OPT) -load $(TRACEATLAS_ROOT)/lib/AtlasPasses.so -Instance $< -o $@

$(SOURCE).lastwriter.bc : $(SOURCE).bc
	$(OPT) -load $(TRACEATLAS_ROOT)/lib/AtlasPasses.so -LastWriter $< -o $@

$(SOURCE).markov.native : $(SOURCE).markov.bc
	$(CXX) $(OPFLAG) $(DEBUG) $(LLD) $(D_LINKS) $(TRACEATLAS_ROOT)/lib/libAtlasBackend.a $< -o $@

$(SOURCE).instance.native : $(SOURCE).instance.bc
	$(CXX) $(OPFLAG) $(DEBUG) $(LLD) $(D_LINKS) $(TRACEATLAS_ROOT)/lib/libAtlasBackend.a $< -o $@

$(SOURCE).hotcode.native : $(SOURCE).hotcode.bc
	$(CXX) $(OPFLAG) $(DEBUG) $(LLD) $(D_LINKS) $(TRACEATLAS_HC_ROOT)/lib/libAtlasBackend.a $< -o $@

$(SOURCE).lastwriter.native : $(SOURCE).lastwriter.bc
	$(CXX) $(OPFLAG) $(DEBUG) $(LLD) $(D_LINKS) $(TRACEATLAS_ROOT)/lib/libAtlasBackend.a $< -o $@

$(SOURCE).bin : $(SOURCE).markov.native
	BLOCK_FILE=BlockInfo_$(SOURCE).json MARKOV_FILE=$(SOURCE).bin ./$< $(RARGS)

$(SOURCE).hotcode.bin : $(SOURCE).hotcode.native
	BLOCK_FILE=BlockInfo_$(SOURCE).hotcode.json MARKOV_FILE=$(SOURCE).hotcode.bin ./$< $(RARGS)

kernel_$(SOURCE).json : $(SOURCE).bin
	$(TRACEATLAS_ROOT)/bin/newCartographer -i $< -b $(SOURCE).bc -bi BlockInfo_$(SOURCE).json -d dot_$(SOURCE).dot -o $@

kernel_$(SOURCE).hotcode.json : $(SOURCE).hotcode.bin
	$(TRACEATLAS_HC_ROOT)/bin/newCartographer -h -i $< -b $(SOURCE).bc -bi BlockInfo_$(SOURCE).hotcode.json -d dot_$(SOURCE).hotcode.dot -o $@

Instance_$(SOURCE).json : $(SOURCE).instance.native kernel_$(SOURCE).json
	KERNEL_FILE=kernel_$(SOURCE).json INSTANCE_FILE=$@ ./$< $(RARGS)

lastwriter.dot : $(SOURCE).lastwriter.native Instance_$(SOURCE).json
	INSTANCE_FILE=Instance_$(SOURCE).json ./$< $(RARGS)

# regular tik
tik_$(SOURCE).bc : kernel_$(SOURCE).json $(SOURCE).bc
	$(TRACEATLAS_ROOT)bin/tik -S -j $^ -o $@

ts_$(SOURCE).bc : tik_$(SOURCE).bc $(SOURCE).bc
	$(TRACEATLAS_ROOT)bin/tikSwap -S -t $< -b $(SOURCE).bc -o $@

ts_$(SOURCE).exec : ts_$(SOURCE).bc tik_$(SOURCE).bc
	$(CXX) $(OPFLAG) $^ -o $@

ts_$(SOURCE)_run : ts_$(SOURCE).exec
	./$< $(RARGS)

# tik with polly
tik_polly_$(SOURCE).bc : tik_$(SOURCE).bc
	$(C) $(LDFLAGS) $(OPFLAG) $(CFLAGS) $(POLLYFLAGS) -S $(LIBRARIES) $< -o $@

ts_polly_$(SOURCE).bc : tik_polly_$(SOURCE).bc $(SOURCE).bc
	$(TRACEATLAS_ROOT)bin/tikSwap -S -t $< -b $(SOURCE).bc -o $@

ts_polly_$(SOURCE).exec : ts_polly_$(SOURCE).bc tik_polly_$(SOURCE).bc
	$(CXX) $(OPFLAG) $^ -o $@

ts_polly_$(SOURCE)_run : ts_polly_$(SOURCE).exec
	./$< $(RARGS)

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
elf : $(SOURCE)$(SUFFIX)
	$(C) $(LLD) $(INCLUDE) $(D_LINKS) $(OPFLAG) $(DEBUG) $(CFLAGS) $(LIBRARIES) $< -o $(SOURCE).elf

run : elf
	./$(SOURCE).elf $(RARGS)

elf_polly : $(SOURCE)$(SUFFIX)
	$(C) $(LLD) $(LDFLAGS) $(INCLUDE) $(OPFLAG) $(DEBUG) $(CFLAGS) $(LIBRARIES) $< -o $(SOURCE).elf_polly.bc
	$(CC) $(LLD) $(INCLUDE) $(D_LINKS) -mllvm -polly $(OPFLAG)  $(POLLY_SHOW) $(POLLY_NONAFFINE) $(LIBRARIES) $(SOURCE).elf_polly.bc -o $(SOURCE).elf_polly

run_polly : elf_polly
	./$(SOURCE).elf_polly $(RARGS)

gprof_$(SOURCE).elf : $(SOURCE)$(SUFFIX)
	$(GC) $(INCLUDE) $(OPFLAG) $(DEBUG) -c -pg -Wno-unused-result  $< -o gprof_$(SOURCE).obj
	$(GC) -pg gprof_$(SOURCE).obj $(D_LINKS) -o $@
	./$@ $(RARGS)

gprof : gprof_$(SOURCE).elf
	gprof -l ./$< $(RARGS)

gcov_$(SOURCE).elf : $(SOURCE)$(SUFFIX)
	$(GXX) $(INCLUDE) $(OPFLAG) $(DEBUG) $(D_LINKS) -Wno-unused-result -fprofile-arcs -ftest-coverage -fPIC $< -o $@

gcov : gcov_$(SOURCE).elf
	./$<
	gcov --all-blocks --branch-probabilities --branch-counts --display-progress --function-summaries $<

perf : elf
	sudo perf stat -d ./BilateralFilter.elf

# can't figure out where oprofile puts the program stdout, so I have to use time -p for now
operf : elf
	sudo time -p operf ./$(SOURCE).elf
	opreport --exclude-dependent --demangle=smart --symbols --threshold=1 > opreport.out

.PHONY:

clean:
	rm -rf *.bc* *.ll *.tr* *.bin *.json *.exec *.elf* *.native *.dot .elf *.obj *.gcda *.gcno *.gcov *.log *.data *.out

clean_oprofile:
	sudo rm -rf oprofile_data
