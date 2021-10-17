include ../../Environment.mk

CC=$(LLVM_HALIDE_INSTALL)bin/clang
CXX=$(LLVM_HALIDE_INSTALL)bin/clang++
OPT=$(LLVM_HALIDE_INSTALL)/bin/opt
LLD=-fuse-ld=$(LLVM_HALIDE_INSTALL)bin/ld.lld
LDFLAGS=-flto -Wl,--plugin-opt=emit-llvm

# In order for this variable to work, your run files need to be named
# $(SOURCE)_generate.cpp $(SOURCE)_run.cpp
# and your generator (-g <generator_name>) needs to match this variable
SOURCE=GEMM
SIGMA_S=1.0
SIGMA_R=1.0
INPUT_IMAGE=john.png
OUTPUT_IMAGE=filteredjohn.png
RARGS=$(SIGMA_S) $(SIGMA_R) $(INPUT_IMAGE) $(OUTPUT_IMAGE)

HALIDE=1

# Halide file generation rules
$(SOURCE)_generated.exec : $(SOURCE)_generate.cpp $(HALIDE_INSTALL_PREFIX)share/tools/GenGen.cpp
	$(CXX) $(HALIDE_COMPILE_ARGS) -I$(HALIDE_INSTALL_PREFIX)/include/ -I$(HALIDE_INSTALL_PREFIX)share/tools/ -L$(HALIDE_INSTALL_PREFIX)/lib/ $(HALIDE_D_LINKS) -lHalide $^ -o $@

$(SOURCE)_autoschedule_false_generated: $(SOURCE)_generated.exec
	LD_LIBRARY_PATH=$(HALIDE_INSTALL_PREFIX)lib/ ./$< -o . -g $(SOURCE) -f $@ -e bitcode,h,cpp target=host auto_schedule=false
$(SOURCE)_autoschedule_true_generated: $(SOURCE)_generated.exec
	LD_LIBRARY_PATH=$(HALIDE_INSTALL_PREFIX)lib/ ./$< -o . -g $(SOURCE) -f $@ -e bitcode,h,cpp -p $(HALIDE_INSTALL_PREFIX)lib/libautoschedule_mullapudi2016.so -s Mullapudi2016 target=host auto_schedule=true machine_params=32,16777216,40

# TraceAtlas pipeline rules
$(SOURCE).bc : $(SOURCE)_run.cpp $(SOURCE)_autoschedule_false_generated
	$(CC) $(LLD) $(LDFLAGS) $(HALIDE_INCLUDE) $(CFLAGS) $(^:%_generated=%_generated.bc) -o $@

$(SOURCE).markov.bc : $(SOURCE).bc
	$(OPT) -load $(TRACEATLAS_ROOT)lib/AtlasPasses.so -Markov $< -o $@

$(SOURCE).instance.bc : $(SOURCE).bc
	$(OPT) -load $(TRACEATLAS_ROOT)lib/AtlasPasses.so -Instance $< -o $@

$(SOURCE).markov.native : $(SOURCE).markov.bc $(SOURCE)_autoschedule_false_generated
	$(CXX) -std=c++11 -fuse-ld=lld-9 $(HALIDE_D_LINKS) $(CFLAGS) -I $(HALIDE_INSTALL_PREFIX)/include/ -I$(HALIDE_INSTALL_PREFIX)share/tools/ $(TRACEATLAS_ROOT)lib/libAtlasBackend.a $(^:%_generated=%_generated.bc) -o $@

$(SOURCE).instance.native : $(SOURCE).instance.bc $(SOURCE)_autoschedule_false_generated
	$(CXX) -std=c++11 -fuse-ld=lld-9 $(HALIDE_D_LINKS) $(CFLAGS) -I $(HALIDE_INSTALL_PREFIX)/include/ -I$(HALIDE_INSTALL_PREFIX)share/tools/ $(TRACEATLAS_ROOT)lib/libAtlasBackend.a $(^:%_generated=%_generated.bc) -o $@

$(SOURCE).bin : $(SOURCE).markov.native
	KERNEL_FILE=kernel_$(SOURCE).json BLOCK_FILE=BlockInfo_$(SOURCE).json ./$< $(RARGS)

kernel_$(SOURCE).json : $(SOURCE).bin
	$(TRACEATLAS_ROOT)bin/newCartographer -i $< -b $(SOURCE)_autoscheduole_false_generated.bc -bi BlockInfo_$(SOURCE).json -d $(SOURCE).dot -o $@

Instance_$(SOURCE).bin : $(SOURCE).instance.native
	INSTANCE_FILE=Instance_$(SOURCE).json ./$< $(RARGS)

# Regular rule and run
$(SOURCE).exec : $(SOURCE)_run.cpp $(SOURCE)_autoschedule_false_generated
	$(CXX) -std=c++11 -fuse-ld=lld-9 $(HALIDE_D_LINKS) $(CFLAGS) $(HALIDE_INCLUDE) $(^:%_generated=%_generated.bc) -o $@

run : $(SOURCE).exec
	./$<

.PHONY : clean

clean:
	rm -rf *.bc *.tr* *.native *.elf *.exec *.bin kernel_* BlockInfo_* *.dot *.ll *.a *_generated*
