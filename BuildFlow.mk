
## compiler variables
# LLVM compiler is the default
CC=$(LLVM_INSTALL)bin/clang
CXX=$(LLVM_INSTALL)bin/clang++
OPT=$(LLVM_INSTALL)bin/opt
LLD=--ld-path=$(LLVM_INSTALL)bin/ld.lld
LDFLAGS?=-flto $(LLD) -Wl,--plugin-opt=emit-llvm
#LDFLAGS?=-c -emit-llvm -g3 -O0 // these flags do not work if linking in static bitcode libraries
# GNU compiler is used for GNU tools
GCC=gcc
GXX=g++
GLD=ld
# CUDA compiler
NVCC?=nvcc
CUPROF?=nvprof

## compile-time configuration flags
# C and C++ flags
CFLAGS?=
CXXFLAGS?=
# optimization flag passed to CC/CXX. Defaults to -O1 because that's what Cyclebite-Template has the best compliance with.
OPFLAG?=-O1
# debug flag passed to CC/CXX. Defaults to all possible debug symbols because this allows Cyclebite-Template annotate tasks
DEBUG?=-g3

## Source file configuration variables
# name of the source file with main in it
SOURCE?=test
# suffix of the file with main in it (can be .c, .cpp, or .cu)
SUFFIX?=.c
# extra path to find this file (relative path from the relative Makefile)
SOURCE_PATH?=
# extra sources to compile
ADDSOURCE?=
# extra sources for compiling Halide
ADDSOURCE_GENERATE?=
# static libraries for compilation phase
LIBRARIES?=
# path to any special dynamic libraries. This should only be a path and contain no white spaces anywhere. For multiple paths, separate with a colon ex. D_LINKS_PATH=/path/to/first/:/path/to/second/
D_LINKS_PATH?=$(LLVM_INSTALL)lib/
# dynamic links to use in the link phase
D_LINKS += -L$(D_LINKS_PATH) 
# include paths for compilation phase. The timinglib header is automatically appended to save redundant stuff in Makefiles
INCLUDE+= -I$(ALGORITHMS_DIR)/TimingLib/

## Runtime variables
# environment variables to set before running a binary. LD_LIBRARY_PATH cannot be in this variable
BIN_VARS?=
# this concatenates all dynamic library paths into LD_LIBRARY_PATH
D_PATH=LD_LIBRARY_PATH="$(SO_PATH):$(D_LINKS_PATH)"
# concatenates all environment variables for running a binary together into a single string
BIN_ENV=$(BIN_VARS) $(D_PATH)
# runtime args to pass to the binary
RARGS?=

# sets the compiler based on the suffix of the main() source file
ifeq ($(SUFFIX),.c)
	C=$(CC)
	GC=$(GCC)
else ifeq ($(SUFFIX),.cpp)
	C=$(CXX)
	GC=$(GXX)
else
	C=$(NVCC)
	GC=$(NVCC)
endif

# TimingLib benchmarking parameters
# samples are number of samples taken of the program. A sample is a trial of TIMINGLIB_ITERATIONS averaged together
TIMINGLIB_SAMPLES?=1
# iterations are number of runs of a program averages together to form a sample
TIMINGLIB_ITERATIONS?=1
CFLAGS += -DTIMINGLIB_SAMPLES=$(TIMINGLIB_SAMPLES) -DTIMINGLIB_ITERATIONS=$(TIMINGLIB_ITERATIONS)

## LLVM-Polly flags
# possibly helpful link: https://groups.google.com/g/polly-dev/c/k5s4dRiZ8rc?pli=1
# polly pass flags
#POLLY_OPT_FLAGS+=-polly-simplify -polly-optree -polly-delicm -polly-simplify -polly-prune-unprofitable -polly-use-llvm-names -polly-export-jscop -polly-process-unprofitable
POLLY_OPT_FLAGS+=-polly-simplify -polly-optree -polly-delicm -polly-simplify -polly-prune-unprofitable -polly-opt-isl -polly-codegen
# turn polly on in compilation pass
POLLY_C_FLAGS+=-mllvm -polly
# have polly output a bunch of dots that it then attempts to open with libreoffice
POLLY_SHOW?=
#POLLY_SHOW?=-mllvm -polly-show-only
# set this to blank if you don't want polly to consider non-affine structures
POLLY_NONAFFINE=-mllvm -polly-allow-nonaffine -mllvm -polly-allow-nonaffine-branches -mllvm -polly-allow-nonaffine-loops
# maximizes vector code generation
POLLY_VECTORIZE=-mllvm -polly-vectorizer=stripmine
# turns on omp code generation and parallelization
POLLY_THREADS?=1
POLLY_PARALLEL=-mllvm -polly-parallel -lgomp -mllvm -polly-num-threads=$(POLLY_THREADS) -fopenmp
# contains all flags that will be passed to polly opt pass
POLLY_C_FLAGS+=$(POLLY_SHOW) $(POLLY_NONAFFINE) $(POLLY_VECTORIZE) $(POLLY_PARALLEL)
# contains all flags that will be passed to clang for polly optimization
POLLY_CLANG_FLAGS = -mllvm -polly $(POLLY_NONAFFINE) $(POLLY_VECTORIZE) $(POLLY_PARALLEL) $(POLLY_SHOW)
## breakdown polly transformation steps
# transforms the input program to a canonical form polly can understand
POLLY_OPTFLAGS1=-S -polly-canonicalize
# print detected scops
POLLY_OPTFLAGS2.0=-polly-use-llvm-names -polly-allow-nonaffine-loops -polly-allow-nonaffine-branches -basicaa -polly-scops -analyze
POLLY_OPTFLAGS2.1=-polly-process-unprofitable 
# Highlight detected scops in the CFG of the program
POLLY_OPTFLAGS3=-polly-use-llvm-names -basicaa#-view-scops # -disable-output

## Rules
# default rule runs the entire Cyclebyte pipeline, through the kernel grammar tool
all : KernelGrammar_$(SOURCE).json

# Halide generator rules
# In order for this variable to work, your run files need to be named
# $(SOURCE)_generate.cpp $(SOURCE)_run.cpp
# and your generator (-g <generator_name>) needs to match this variable
# See GEMM/Halide/ and its Makefile for an example
$(SOURCE)_generated.exec : $(SOURCE_PATH)$(SOURCE)_generate.cpp $(HALIDE_INSTALL_PREFIX)share/tools/GenGen.cpp $(ADDSOURCE_GENERATE)
	$(CXX) $(HALIDE_COMPILE_ARGS) $(DEBUG) $(OPFLAG) $(INCLUDE) $(HALIDE_INCLUDE) $(CFLAGS) $(CXXFLAGS) -L$(HALIDE_INSTALL_PREFIX)lib/ $(HALIDE_D_LINKS) -lHalide $^ -o $@

ifeq ($(HALIDE_AUTOSCHEDULE),1)
$(SOURCE)_autoschedule_true_generated.bc $(SOURCE)_autoschedule_true_generated.h $(SOURCE)_autoschedule_true_generated.halide_generated.cpp : $(SOURCE)_generated.exec
	LD_LIBRARY_PATH=$(HALIDE_INSTALL_PREFIX)lib/ ./$< -o . -g $(SOURCE) -f $(SOURCE)_autoschedule_true_generated -e bitcode,h,cpp -p $(HALIDE_INSTALL_PREFIX)lib/libautoschedule_mullapudi2016.so -s Mullapudi2016 target=host auto_schedule=true machine_params=32,16777216,40
endif
$(SOURCE)_autoschedule_false_generated.bc $(SOURCE)_autoschedule_false_generated.h $(SOURCE)_autoschedule_false_generated.halide_generated.cpp : $(SOURCE)_generated.exec
	LD_LIBRARY_PATH=$(HALIDE_INSTALL_PREFIX)lib/ ./$< -o . -g $(SOURCE) -f $(SOURCE)_autoschedule_false_generated -e bitcode,h,cpp target=host

# Halide needs to be built a special way
ifeq ($(HALIDE).$(HALIDE_AUTOSCHEDULE),1.1)
$(SOURCE).bc : $(SOURCE)_run.cpp $(SOURCE)_autoschedule_true_generated.bc $(SOURCE)_autoschedule_false_generated.bc $(ADDSOURCE)
	$(C) $(LDFLAGS) $(OPFLAG) $(DEBUG) $(HALIDE_INCLUDE) $(INCLUDE) $(CFLAGS) $(CXXFLAGS) $(^:%_generated=%_generated.bc) -o $@
else ifeq ($(HALIDE).$(HALIDE_AUTOSCHEDULE),1.0)
$(SOURCE).bc : $(SOURCE)_run.cpp $(SOURCE)_autoschedule_false_generated.bc $(ADDSOURCE)
	$(C) $(LDFLAGS) $(OPFLAG) $(DEBUG) $(HALIDE_INCLUDE) $(INCLUDE) $(CFLAGS) $(CXXFLAGS) $(^:%_generated=%_generated.bc) -o $@
else
$(SOURCE).bc : $(SOURCE_PATH)$(SOURCE)$(SUFFIX) $(ADDSOURCE)
	$(C) $(LDFLAGS) $(OPFLAG) $(DEBUG) $(INCLUDE) $(LIBRARIES) $(CFLAGS) $(CXXFLAGS) $^ -o $@
endif

# Cyclebyte pipeline rules
$(SOURCE).markov.bc : $(SOURCE).bc
	LOOP_FILE=Loopfile_$(SOURCE).json $(OPT) --load-pass-plugin $(CYCLEBITE_ROOT)lib/MarkovPass.so --passes Markov $< -o $@
#	LOOP_FILE=Loopfile_$(SOURCE).json $(OPT) -enable-new-pm=0 -load $(CYCLEBITE_ROOT)lib/AtlasPasses.so --Markov $< -o $@

$(SOURCE).memory.bc : $(SOURCE).bc
	$(OPT) --load-pass-plugin $(CYCLEBITE_ROOT)lib/MemoryPass.so --passes Memory $< -o $@
#	$(OPT) -enable-new-pm=0 -load $(CYCLEBITE_ROOT)lib/AtlasPasses.so --Memory $< -o $@

$(SOURCE).markov.native : $(SOURCE).markov.bc
	$(CXX) $(OPFLAG) $(DEBUG) $(LLD) $(D_LINKS) $(CYCLEBITE_ROOT)lib/libAtlasBackend.so $< -o $@

$(SOURCE).memory.native : $(SOURCE).memory.bc
	$(CXX) $(OPFLAG) $(DEBUG) $(LLD) $(D_LINKS) $(CYCLEBITE_ROOT)lib/libAtlasBackend.so $< -o $@

$(SOURCE).bin : $(SOURCE).markov.native
	$(BIN_ENV) BLOCK_FILE=BlockInfo_$(SOURCE).json MARKOV_FILE=$(SOURCE).bin ./$< $(RARGS)

kernel_$(SOURCE).json : $(SOURCE).bin
	LD_LIBRARY_PATH=$(SO_PATH) $(CYCLEBITE_ROOT)bin/newCartographer -i $< -b $(SOURCE).bc -bi BlockInfo_$(SOURCE).json -d dot_$(SOURCE).dot -h -l Loopfile_$(SOURCE).json -o $@

instance_$(SOURCE).json : $(SOURCE).memory.native kernel_$(SOURCE).json
	$(BIN_ENV) INSTANCE_FILE=instance_$(SOURCE).json TASKGRAPH_FILE=TaskGraph_$(SOURCE).dot MEMORY_DOTFILE=Memory_$(SOURCE).dot CSV_FILE=MemoryFootprints_$(SOURCE).csv KERNEL_FILE=kernel_$(SOURCE).json ./$< $(RARGS)

KernelGrammar_$(SOURCE).json : instance_$(SOURCE).json
	LD_LIBRARY_PATH=$(SO_PATH) $(CYCLEBITE_ROOT)bin/KernelGrammar -i $< -k kernel_$(SOURCE).json -b $(SOURCE).bc -bi BlockInfo_$(SOURCE).json -p $(SOURCE).bin -o $@

$(SOURCE).annotated_omp.native : KernelGrammar_$(SOURCE).json $(ADDSOURCE)
	$(C) -fopenmp $(LLD) $(INCLUDE) $(D_LINKS) $(OPFLAG) $(DEBUG) $(LIBRARIES) $(CFLAGS) $(CXXFLAGS) $(SOURCE).annotated_omp$(SUFFIX) $(ADDSOURCE) -o $@

run_annotated : $(SOURCE).annotated_omp.native
	$(BIN_ENV) ./$< $(RARGS)

# render the resulting DOT files with graphviz install
KDFG_DOTS  = $(wildcard DFG_kernel*.dot)
KDFG_NAMES = $(patsubst %.dot,%,$(KDFG_DOTS))
KDFG_ENUM  = $(foreach d,$(KDFG_NAMES),$d.svg)
define DOT_RENDER_RULE = 
$(1).svg : 
	$(DOT) -Tsvg -o $(1).svg $(1).dot ; $(DOT_RENDER) $(1).svg
endef

dots : $(KDFG_ENUM) KernelGrammar_$(SOURCE).json
$(foreach d,$(KDFG_NAMES), $(eval $(call DOT_RENDER_RULE,$d)) )

# map tasks back to the source code with debug symbols
SourceMap_$(SOURCE).json : kernel_$(SOURCE).json
	$(CYCLEBITE_ROOT)bin/kernelSourceMapper -i $(SOURCE).bc -k $< -o SourceMap_$(SOURCE)_kernel.json
	$(CYCLEBITE_ROOT)bin/kernelSourceMapper -i $(SOURCE).bc -k instance_$(SOURCE).json -o SourceMap_$(SOURCE)_instance.json

# Precision Analysis pass
$(SOURCE).precision.bc : $(SOURCE).bc
	$(OPT) --load-pass-plugin $(CYCLEBITE_ROOT)lib/AtlasPasses.so --passes Precision $< -o $@
#	$(OPT) -enable-new-pm=0 -load $(CYCLEBITE_ROOT)lib/AtlasPasses.so --Precision $< -o $@

$(SOURCE).precision.native : $(SOURCE).precision.bc
	$(CXX) $(OPFLAG) $(DEBUG) $(LLD) $(D_LINKS) $(CYCLEBITE_ROOT)lib/libAtlasBackend.so $< -o $@

precision.json : $(SOURCE).precision.native instance_$(SOURCE).json
	$(BIN_ENV) KERNEL_FILE=instance_$(SOURCE).json ./$< $(RARGS)

# regular tik
tik_$(SOURCE).bc : kernel_$(SOURCE).json $(SOURCE).bc
	$(CYCLEBITE_ROOT)bin/tik -S -j $^ -o $@

ts_$(SOURCE).bc : tik_$(SOURCE).bc $(SOURCE).bc
	$(CYCLEBITE_ROOT)bin/tikSwap -S -t $< -b $(SOURCE).bc -o $@

ts_$(SOURCE).exec : ts_$(SOURCE).bc tik_$(SOURCE).bc
	$(CXX) $(OPFLAG) $^ -o $@

ts_$(SOURCE)_run : ts_$(SOURCE).exec
	./$< $(RARGS)

# tik with polly
tik_polly_$(SOURCE).bc : tik_$(SOURCE).bc
	$(C) $(LDFLAGS) $(OPFLAG) $(CFLAGS) $(CXXFLAGS) $(POLLYFLAGS) -S $(LIBRARIES) $< -o $@

ts_polly_$(SOURCE).bc : tik_polly_$(SOURCE).bc $(SOURCE).bc
	$(CYCLEBITE_ROOT)bin/tikSwap -S -t $< -b $(SOURCE).bc -o $@

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

# polygeist test rule
cgeist : $(SOURCE_PATH)$(SOURCE)$(SUFFIX) $(ADDSOURCE)
	$(CGEIST) $^ -o $(SOURCE).cgeist

# builds the source code into elf form, no instrumentation
# Halide needs to be built a special way
ifeq ($(HALIDE).$(HALIDE_AUTOSCHEDULE),1.1)
$(SOURCE).elf : $(SOURCE)_run.cpp $(SOURCE)_autoschedule_true_generated.bc $(SOURCE)_autoschedule_false_generated.bc $(ADDSOURCE)
	$(C) $(LLD) $(HALIDE_INCLUDE) $(INCLUDE) $(D_LINKS) $(HALIDE_D_LINKS) $(OPFLAG) $(DEBUG) $(CFLAGS) $(CXXFLAGS) $(^:%_generated=%_generated.bc) -o $@
else ifeq ($(HALIDE).$(HALIDE_AUTOSCHEDULE),1.0)
$(SOURCE).elf : $(SOURCE)_run.cpp $(SOURCE)_autoschedule_false_generated.bc $(ADDSOURCE)
	$(C) $(LLD) $(HALIDE_INCLUDE) $(INCLUDE) $(D_LINKS) $(HALIDE_D_LINKS) $(OPFLAG) $(DEBUG) $(CFLAGS) $(CXXFLAGS) $(^:%_generated=%_generated.bc) -o $@
else ifeq ($(SUFFIX),.cu)
$(SOURCE).elf : $(SOURCE)$(SUFFIX) $(ADDSOURCE)
	$(C) $(INCLUDE) $(D_LINKS) $(OPFLAG) $(LIBRARIES) $(CFLAGS) $(CXXFLAGS) $^ -o $@ 
else
$(SOURCE).elf : $(SOURCE)$(SUFFIX) $(ADDSOURCE)
	$(C) $(LLD) $(INCLUDE) $(D_LINKS) $(OPFLAG) $(DEBUG) $(LIBRARIES) $(CFLAGS) $(CXXFLAGS) $^ -o $@ 
endif

run : $(SOURCE).elf
	$(BIN_ENV) ./$< $(RARGS)

gdb : $(SOURCE).elf
	gdb --args $< $(RARGS)

ifeq ($(HALIDE).$(HALIDE_AUTOSCHEDULE),1.1)
$(SOURCE).elf_polly : $(SOURCE)_run.cpp $(SOURCE)_autoschedule_true_generated.bc $(SOURCE)_autoschedule_false_generated.bc $(ADDSOURCE)
	$(C) $(LLD) $(HALIDE_INCLUDE) $(INCLUDE) $(D_LINKS) $(HALIDE_D_LINKS) $(OPFLAG) $(DEBUG) $(CFLAGS) $(CXXFLAGS) $(POLLY_CLANG_FLAGS) $(^:%_generated=%_generated.bc) -o $@
else ifeq ($(HALIDE).$(HALIDE_AUTOSCHEDULE),1.0)
$(SOURCE).elf_polly : $(SOURCE)_run.cpp $(SOURCE)_autoschedule_false_generated.bc $(ADDSOURCE)
	$(C) $(LLD) $(HALIDE_INCLUDE) $(INCLUDE) $(D_LINKS) $(HALIDE_D_LINKS) $(OPFLAG) $(DEBUG) $(CFLAGS) $(CXXFLAGS) $(^:%_generated=%_generated.bc) -o $@
else
$(SOURCE).elf_polly : $(SOURCE)$(SUFFIX) $(ADDSOURCE)
	$(C) $(LLD) $(INCLUDE) $(D_LINKS) $(OPFLAG) $(DEBUG) $(LIBRARIES) $(CFLAGS) $(CXXFLAGS) $(POLLY_CLANG_FLAGS) $^ -o $@
endif

#$(SOURCE).bc_polly : $(SOURCE).bc
#	$(OPT) $(POLLY_OPT_FLAGS) $< -o $@
#elf_polly : $(SOURCE).bc_polly
#	$(C) $(LLD) $(D_LINKS) $(OPFLAG) $(DEBUG) $(POLLY_C_FLAGS) $< -o $(SOURCE).elf_polly

run_polly : $(SOURCE).elf_polly
	$(BIN_ENV) ./$< $(RARGS)

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

ll : $(SOURCE).markov.bc $(SOURCE).memory.bc #$(SOURCE).precision.bc
	$(DIS) $(SOURCE).markov.bc
	$(DIS) $(SOURCE).memory.bc
	#$(DIS) $(SOURCE).precision.bc

cuprof : $(SOURCE).elf
	$(CUPROF) $<

.PHONY:

clean:
	rm -rf *.bc* *.ll *.tr* *.bin *.json *.exec *.elf* *.native *.dot *.dot_taskonly *.obj *.gcda *.gcno *.gcov *.log *.data *.out *_generated* *_output.* *.raw MemoryFootprint*.csv *.jscop

clean_oprofile:
	sudo rm -rf oprofile_data

# [BW] these passes are deprecated as of 7/01/22
#$(SOURCE).instance.bc : $(SOURCE).bc
#	$(OPT) -load $(CYCLEBITE_ROOT)lib/AtlasPasses.so -Instance $< -o $@

#$(SOURCE).lastwriter.bc : $(SOURCE).bc
#	$(OPT) -load $(CYCLEBITE_ROOT)lib/AtlasPasses.so -LastWriter $< -o $@

#$(SOURCE).instance.native : $(SOURCE).instance.bc
#	$(CXX) $(OPFLAG) $(DEBUG) $(LLD) $(D_LINKS) $(CYCLEBITE_ROOT)lib/libAtlasBackend.so $< -o $@

#$(SOURCE).lastwriter.native : $(SOURCE).lastwriter.bc
#	$(CXX) $(OPFLAG) $(DEBUG) $(LLD) $(D_LINKS) $(CYCLEBITE_ROOT)lib/libAtlasBackend.so $< -o $@

#Instance_$(SOURCE).json : $(SOURCE).instance.native kernel_$(SOURCE).json
#	LD_LIBRARY_PATH=$(SO_PATH) KERNEL_FILE=kernel_$(SOURCE).json INSTANCE_FILE=$@ ./$< $(RARGS)

#lastwriter_$(SOURCE).dot : $(SOURCE).lastwriter.native Instance_$(SOURCE).json
#	LD_LIBRARY_PATH=$(SO_PATH) INSTANCE_FILE=Instance_$(SOURCE).json ./$< $(RARGS)


