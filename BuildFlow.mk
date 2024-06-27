
## compiler variables
# LLVM compiler is the default
CC=$(LLVM_INSTALL)bin/clang
CXX=$(LLVM_INSTALL)bin/clang++
OPT=$(LLVM_INSTALL)bin/opt
LLD=--ld-path=$(LLVM_INSTALL)bin/ld.lld
LDFLAGS?=-flto -Wl,--plugin-opt=emit-llvm $(LLD) 
#LDFLAGS?=-c -emit-llvm -g3 -O0 // these flags do not work if linking in static bitcode libraries
# GNU compiler is used for GNU tools
GCC=gcc
GXX=g++
GLD=ld
# CUDA compiler
NVCC?=nvcc
CUPROF?=nvprof
# Polygeist stuff
PC?=$(POLYGEIST_INSTALL)bin/cgeist
#POLYMEROPT=$(POLYGEIST_INSTALL)bin/polygeist-opt
POLYMEROPT=$(POLYGEIST_INSTALL)bin/polymer-opt
MLIROPT?=$(POLYGEIST_INSTALL)bin/mlir-opt
MLIRTRANSLATE?=$(POLYGEIST_INSTALL)bin/mlir-translate

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
# some libraries require that some archives are treated differently than others (e.g., pytorch requires specific flags for specific archives)
# thus we have a special variable for the flag belonging to those second archives and a variable for those second archives
ARCHIVE_FLAGS?=
LIBRARIES2?=
# path to any special dynamic libraries. This should only be a path and contain no white spaces anywhere. For multiple paths, separate with a colon ex. D_LINKS_PATH=/path/to/first/:/path/to/second/
D_LINKS_PATH?=$(LLVM_INSTALL)lib/
# dynamic links to use in the link phase
D_LINKS += -L$(D_LINKS_PATH) 
# include paths for compilation phase. The timinglib header is automatically appended to save redundant stuff in Makefiles
INCLUDE+= -I$(ALGORITHMS_DIR)/TimingLib/ -I$(ALGORITHMS_DIR)/inc/

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
POLLY_PARALLEL=-mllvm -polly-parallel -lgomp -mllvm -polly-num-threads=$(POLLY_THREADS) -mllvm -polly-omp-backend=LLVM -mllvm -polly-scheduling=static -fopenmp  
# contains all flags that will be passed to polly opt pass
POLLY_C_FLAGS+=$(POLLY_SHOW) $(POLLY_NONAFFINE) $(POLLY_VECTORIZE) $(POLLY_PARALLEL)
# contains all flags that will be passed to clang for polly optimization
POLLY_CLANG_FLAGS = -mllvm -polly -ffast-math -ffinite-math-only -funsafe-math-optimizations -fsave-optimization-record $(POLLY_NONAFFINE) $(POLLY_VECTORIZE) $(POLLY_PARALLEL) $(POLLY_SHOW)
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

# choice of autoschedulers as of Halide 16: mullapudi2016, adams2019, anderson2021 (GPU only) 
# check your cuda_capability_## parameter at https://developer.nvidia.com/cuda-gpus#compute
# the autoscheduler library needs to be lower case
HALIDE_AUTOSCHEDULER_LIB=$(shell echo $(HALIDE_AUTOSCHEDULER) | tr A-Z a-z)
# the halide target should be the host gpu (for Anderson2021) and host for everything else
ifeq ($(HALIDE_AUTOSCHEDULER),Anderson2021)
	HALIDE_TARGET=host-cuda-cuda_capability_86
else
	HALIDE_TARGET=host
endif
ifeq ($(HALIDE_AUTOSCHEDULE),1)
$(SOURCE)_autoschedule_true_generated.bc $(SOURCE)_autoschedule_true_generated.h $(SOURCE)_autoschedule_true_generated.halide_generated.cpp : $(SOURCE)_generated.exec
	LD_LIBRARY_PATH=$(HALIDE_INSTALL_PREFIX)lib/ ./$< -o . -g $(SOURCE) -f $(SOURCE)_autoschedule_true_generated -e bitcode,h,cpp -p $(HALIDE_INSTALL_PREFIX)lib/libautoschedule_$(HALIDE_AUTOSCHEDULER_LIB).so autoscheduler=$(HALIDE_AUTOSCHEDULER) target=$(HALIDE_TARGET)
endif
$(SOURCE)_autoschedule_false_generated.bc $(SOURCE)_autoschedule_false_generated.h $(SOURCE)_autoschedule_false_generated.halide_generated.cpp : $(SOURCE)_generated.exec
	LD_LIBRARY_PATH=$(HALIDE_INSTALL_PREFIX)lib/ ./$< -o . -g $(SOURCE) -f $(SOURCE)_autoschedule_false_generated -e bitcode,h,cpp target=host

# Halide needs to be built a special way
ifeq ($(HALIDE).$(HALIDE_AUTOSCHEDULE),1.1)
$(SOURCE).bc : $(SOURCE)_run.cpp $(SOURCE)_autoschedule_true_generated.bc $(SOURCE)_autoschedule_false_generated.bc $(ADDSOURCE)
	$(C) $(LDFLAGS) $(OPFLAG) $(DEBUG) $(HALIDE_INCLUDE) $(INCLUDE) -DHALIDE_AUTOSCHEDULE=$(HALIDE_AUTOSCHEDULE) $(CFLAGS) $(CXXFLAGS) $(^:%_generated=%_generated.bc) -o $@
else ifeq ($(HALIDE).$(HALIDE_AUTOSCHEDULE),1.0)
$(SOURCE).bc : $(SOURCE)_run.cpp $(SOURCE)_autoschedule_false_generated.bc $(ADDSOURCE)
	$(C) $(LDFLAGS) $(OPFLAG) $(DEBUG) $(HALIDE_INCLUDE) $(INCLUDE) $(CFLAGS) $(CXXFLAGS) $(^:%_generated=%_generated.bc) -o $@
else
$(SOURCE).bc : $(SOURCE_PATH)$(SOURCE)$(SUFFIX) $(ADDSOURCE)
	$(C) $(LDFLAGS) $(OPFLAG) $(DEBUG) $(INCLUDE) $(CFLAGS) $(CXXFLAGS) $^ $(LIBRARIES) $(ARCHIVE_FLAGS) $(LIBRARIES2) -o $@
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
	$(C) -fopenmp $(LLD) $(INCLUDE) $(D_LINKS) $(OPFLAG) $(DEBUG) $(CFLAGS) $(CXXFLAGS) $(LIBRARIES) $(ARCHIVE_FLAGS) $(LIBRARIES2) $(SOURCE).annotated_omp$(SUFFIX) $(ADDSOURCE) -o $@

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
	$(C) $(LDFLAGS) $(OPFLAG) $(POLLYFLAGS) -S $(CFLAGS) $(CXXFLAGS) $(LIBRARIES) $(ARCHIVE_FLAGS) $(LIBRARIES2) $< -o $@

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
# add -S to see the generated MLIR
# [BW] 2024-04-09 the polygeist compiler works with just the first step on input applications from polybench, but it doesn't produce valid MLIR (memref has an llvm.struct element type, which is not allowed: https://mlir.llvm.org/doxygen/MemRefToLLVM_8cpp_source.html line 1190)
#      -> without exporting valid MLIR, we can't parallelize its output, and when we run the cgeist output alone, we get performance that under-performs the clang17 front-end on -O3 -g0
#      -> also, cgeist does worse for higher optimization levels (optimal op level seems to be -O1)
$(SOURCE).cgeist.native : $(SOURCE_PATH)$(SOURCE)$(SUFFIX) $(ADDSOURCE)
	LD_LIBRARY_PATH=/home/ben/Builds/Polygeist/build/tools/polygeist/pluto/install/lib/ $(CGEIST) --raise-scf-to-affine --c-style-memref --openmp-opt --polyhedral-opt -fopenmp -lomp $(INCLUDE) $(D_LINKS) $(OPFLAG) $(LIBRARIES) $(CFLAGS) $(CXXFLAGS) $^ -o $(SOURCE).cgeist.native
	#LD_LIBRARY_PATH=/home/ben/Builds/Polygeist/build/tools/polygeist/pluto/install/lib/ $(CGEIST) --raise-scf-to-affine --memref-abi --c-style-memref -S $(INCLUDE) $(D_LINKS) $(OPFLAG) $(CFLAGS) $(CXXFLAGS) $(LIBRARIES) $(ARCHIVE_FLAGS) $(LIBRARIES2) $^ -o $(SOURCE).cgeist.mlir
	# this command breaks polygeist-op -> $(POLYGEIST_INSTALL)bin/polygeist-opt --convert-polygeist-to-llvm $(SOURCE).cgeist.mlir -o $(SOURCE).simplify.mlir
	#LD_LIBRARY_PATH=/home/ben/Builds/Polygeist/build/tools/polygeist/pluto/install/lib/ $(POLYMEROPT) -allow-unregistered-dialect --demote-loop-reduction --extract-scop-stmt --pluto-opt="parallelize=1" --inline --canonicalize $(SOURCE).cgeist.mlir -o $(SOURCE).polymerpar.mlir
	#$(MLIROPT) -mem2reg -detect-reduction -mem2reg -canonicalize -affine-parallelize -lower-affine -convert-scf-to-openmp -convert-scf-to-std -convert-openmp-to-llvm $(SOURCE).polymerpar.mlir -o $(SOURCE).mliropt.mlir
	#$(MLIROPT) -mem2reg -canonicalize -affine-parallelize -lower-affine -convert-scf-to-openmp -convert-scf-to-cf -convert-openmp-to-llvm $(SOURCE).cgeist.mlir -o $(SOURCE).mliropt.mlir
	#$(MLIRTRANSLATE) -mlir-to-llvmir $(SOURCE).mliropt.mlir -o $(SOURCE).polygeist.bc
	#$(MLIRTRANSLATE) -mlir-to-llvmir $(SOURCE).cgeist.mlir -o $(SOURCE).polygeist.bc
	#$(CC) -fopenmp -lomp $(INCLUDE) $(D_LINKS) -O3 -g0 $(CFLAGS) $(CXXFLAGS) $(LIBRARIES) $(ARCHIVE_FLAGS) $(LIBRARIES2) $(SOURCE).polygeist.bc -o $@

run_cgeist : $(SOURCE).cgeist.native
	$(BIN_ENV) ./$< $(RARGS)

# builds the source code into elf form, no instrumentation
# Halide needs to be built a special way
ifeq ($(HALIDE).$(HALIDE_AUTOSCHEDULE),1.1)
$(SOURCE).elf : $(SOURCE)_run.cpp $(SOURCE)_autoschedule_true_generated.bc $(SOURCE)_autoschedule_false_generated.bc $(ADDSOURCE)
	$(C) $(LLD) $(HALIDE_INCLUDE) $(INCLUDE) $(D_LINKS) $(HALIDE_D_LINKS) $(OPFLAG) $(DEBUG) -DHALIDE_AUTOSCHEDULE=$(HALIDE_AUTOSCHEDULE) $(CFLAGS) $(CXXFLAGS) $(^:%_generated=%_generated.bc) -o $@
else ifeq ($(HALIDE).$(HALIDE_AUTOSCHEDULE),1.0)
$(SOURCE).elf : $(SOURCE)_run.cpp $(SOURCE)_autoschedule_false_generated.bc $(ADDSOURCE)
	$(C) $(LLD) $(HALIDE_INCLUDE) $(INCLUDE) $(D_LINKS) $(HALIDE_D_LINKS) $(OPFLAG) $(DEBUG) $(CFLAGS) $(CXXFLAGS) $(^:%_generated=%_generated.bc) -o $@
else ifeq ($(SUFFIX),.cu)
$(SOURCE).elf : $(SOURCE)$(SUFFIX) $(ADDSOURCE)
	$(C) $(INCLUDE) $(D_LINKS) $(OPFLAG) $(CFLAGS) $(CXXFLAGS) $(LIBRARIES) $(ARCHIVE_FLAGS) $(LIBRARIES2) $^ -o $@ 
else
$(SOURCE).elf : $(SOURCE)$(SUFFIX) $(ADDSOURCE)
	$(C) $(LLD) $(INCLUDE) $(D_LINKS) $(OPFLAG) $(DEBUG) $(CFLAGS) $(CXXFLAGS) $(LIBRARIES) $(ARCHIVE_FLAGS) $(LIBRARIES2) $^ -o $@ 
endif

run : $(SOURCE).elf
	$(BIN_ENV) ./$< $(RARGS)

gdb : $(SOURCE).elf
	gdb --args $< $(RARGS)

ifeq ($(HALIDE).$(HALIDE_AUTOSCHEDULE),1.1)
$(SOURCE).elf_polly : $(SOURCE)_run.cpp $(SOURCE)_autoschedule_true_generated.bc $(SOURCE)_autoschedule_false_generated.bc $(ADDSOURCE)
	$(C) $(LLD) $(HALIDE_INCLUDE) $(INCLUDE) $(D_LINKS) $(HALIDE_D_LINKS) $(OPFLAG) $(DEBUG) -DHALIDE_AUTOSCHEDULE=$(HALIDE_AUTOSCHEDULE) $(CFLAGS) $(CXXFLAGS) $(POLLY_CLANG_FLAGS) $(^:%_generated=%_generated.bc) -o $@
else ifeq ($(HALIDE).$(HALIDE_AUTOSCHEDULE),1.0)
$(SOURCE).elf_polly : $(SOURCE)_run.cpp $(SOURCE)_autoschedule_false_generated.bc $(ADDSOURCE)
	$(C) $(LLD) $(HALIDE_INCLUDE) $(INCLUDE) $(D_LINKS) $(HALIDE_D_LINKS) $(OPFLAG) $(DEBUG) $(CFLAGS) $(CXXFLAGS) $(^:%_generated=%_generated.bc) -o $@
else
$(SOURCE).elf_polly : $(SOURCE).bc
	#$(OPT) -basic-aa -polly-use-llvm-names -polly-export-jscop -polly-process-unprofitable -polly-parallel -polly-allow-nonaffine -polly-allow-nonaffine-branches -polly-allow-nonaffine-loops -polly-vectorizer=stripmine $(SOURCE).bc 
	# if you try to read in the jscop, llvm-polly breaks (so you can't use the -polly-import-jscop option) $(OPT) -S $(SOURCE).bc -basic-aa -polly-use-llvm-names -polly-import-jscop -polly-import-jscop-postfix=interchanged+tiled+vector -polly-codegen -polly-parallel -polly-process-unprofitable -polly-allow-nonaffine -polly-allow-nonaffine-branches -polly-allow-nonaffine-loops -polly-vectorizer=stripmine -o $(SOURCE).polly.bc
	# useful for polybench3.2/gemm
	$(OPT) -S $(SOURCE).bc -basic-aa -polly-use-llvm-names -polly-export -polly-export-jscop -polly-codegen -polly-omp-backend=LLVM -polly-parallel -polly-vectorizer=stripmine -polly-process-unprofitable -polly-allow-nonaffine -polly-allow-nonaffine-branches -polly-allow-nonaffine-loops -polly-only-func=kernel_gemm -o $(SOURCE).polly.bc
	#$(OPT) -S $(SOURCE).bc -basic-aa -polly-use-llvm-names -polly-export-jscop -polly-codegen -polly-omp-backend=LLVM -polly-parallel -polly-vectorizer=stripmine -polly-process-unprofitable -polly-allow-nonaffine -polly-allow-nonaffine-branches -polly-allow-nonaffine-loops -o $(SOURCE).polly.bc
	$(C) $(LLD) $(INCLUDE) $(D_LINKS) $(OPFLAG) $(DEBUG) $(CFLAGS) $(CXXFLAGS) $(POLLY_CLANG_FLAGS) $(LIBRARIES) $(ARCHIVE_FLAGS) $(LIBRARIES2) $(SOURCE).polly.bc -o $@
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
	rm -rf *.bc* *.ll *.tr* *.bin *.json *.exec *.elf* *.native *.dot *.dot_taskonly *.obj *.gcda *.gcno *.gcov *.log *.data *.out *_generated* *_output.* *.raw MemoryFootprint*.csv *.jscop *.cgeist *.annotated_omp.c*

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


