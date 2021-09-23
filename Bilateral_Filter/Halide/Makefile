
# LLVM12 install
LLVM_INSTALL_PREFIX=/mnt/heorot-10/bwilli46/LLVM12/install-release/
#LLVM_INSTALL_PREFIX=/usr/lib/llvm-9/

CC=$(LLVM_INSTALL_PREFIX)bin/clang
CXX=$(LLVM_INSTALL_PREFIX)bin/clang++
OPT=$(LLVM_INSTALL_PREFIX)/bin/opt
LDFLAGS=-flto -fuse-ld=$(LLVM_INSTALL_PREFIX)bin/ld.lld -Wl,--plugin-opt=emit-llvm
# TraceAtlas install
TRACEATLAS_ROOT=/home/bwilli46/TraceAtlas/build2/
DASH_DATA=/mnt/nobackup-09/Dash/Data/
CFLAGS+=-DDASH_DATA=\"$(DASH_DATA)\" -std=c++17
# Halide install
HALIDE_INSTALL_PREFIX=/mnt/heorot-10/bwilli46/Halide-install-release/
#HALIDE_COMPILE_ARGS=-g3 -std=c++11 -fno-rtti
HALIDE_COMPILE_ARGS=-g3 -fno-rtti
HALIDE_D_LINKS=-lpthread -ldl -lpng -ljpeg
HALIDE_INCLUDE=\
-I$(HALIDE_INSTALL_PREFIX)include/\
-I$(HALIDE_INSTALL_PREFIX)share/tools

# In order for this variable to work, your run files need to be named
# $(SOURCE)_generate.cpp $(SOURCE)_run.cpp
# and your generator (-g <generator_name>) needs to match this variable
SOURCE=bilateral_filter
OPT_TEST=Instance

SIGMA_S=1.0
SIGMA_R=1.0
INPUT_IMAGE=john.png
OUTPUT_IMAGE=filteredjohn.png

AUTOSCHEDULE=0

# Halide file generation rules
$(SOURCE)_generated.exec : $(SOURCE)_generate.cpp $(HALIDE_INSTALL_PREFIX)share/tools/GenGen.cpp
	$(CXX) $(HALIDE_COMPILE_ARGS) -I$(HALIDE_INSTALL_PREFIX)/include/ -I$(HALIDE_INSTALL_PREFIX)share/tools/ -L$(HALIDE_INSTALL_PREFIX)/lib/ $(HALIDE_D_LINKS) -lHalide $^ -o $@

$(SOURCE)_autoschedule_false_generated: $(SOURCE)_generated.exec
	LD_LIBRARY_PATH=$(HALIDE_INSTALL_PREFIX)lib/ ./$< -o . -g $(SOURCE) -f $@ -e bitcode,h,cpp target=host auto_schedule=false
$(SOURCE)_autoschedule_true_generated: $(SOURCE)_generated.exec
	LD_LIBRARY_PATH=$(HALIDE_INSTALL_PREFIX)lib/ ./$< -o . -g $(SOURCE) -f $@ -e bitcode,h,cpp -p $(HALIDE_INSTALL_PREFIX)lib/libautoschedule_mullapudi2016.so -s Mullapudi2016 target=host auto_schedule=true machine_params=32,16777216,40

# TraceAtlas pipeline rules
$(SOURCE).bc : $(SOURCE)_run.cpp $(SOURCE)_autoschedule_false_generated
	$(CC) $(LDFLAGS) $(HALIDE_INCLUDE) $(CFLAGS) $(^:%_generated=%_generated.bc) -o $@

$(SOURCE).markov.bc : $(SOURCE).bc
	$(OPT) -load $(TRACEATLAS_ROOT)lib/AtlasPasses.so -Markov $< -o $@

$(SOURCE).instance.bc : $(SOURCE).bc
	$(OPT) -load $(TRACEATLAS_ROOT)lib/AtlasPasses.so -Instance $< -o $@

$(SOURCE).markov.native : $(SOURCE).markov.bc $(SOURCE)_autoschedule_false_generated
	$(CXX) -std=c++11 -fuse-ld=lld-9 $(HALIDE_D_LINKS) $(CFLAGS) -I $(HALIDE_INSTALL_PREFIX)/include/ -I$(HALIDE_INSTALL_PREFIX)share/tools/ $(TRACEATLAS_ROOT)lib/libAtlasBackend.a $(^:%_generated=%_generated.bc) -o $@

$(SOURCE).instance.native : $(SOURCE).instance.bc $(SOURCE)_autoschedule_false_generated
	$(CXX) -std=c++11 -fuse-ld=lld-9 $(HALIDE_D_LINKS) $(CFLAGS) -I $(HALIDE_INSTALL_PREFIX)/include/ -I$(HALIDE_INSTALL_PREFIX)share/tools/ $(TRACEATLAS_ROOT)lib/libAtlasBackend.a $(^:%_generated=%_generated.bc) -o $@

$(SOURCE).bin : $(SOURCE).markov.native
	KERNEL_FILE=kernel_$(SOURCE).json BLOCK_FILE=BlockInfo_$(SOURCE).json ./$< $(SIGMA_S) $(SIGMA_R) $(INPUT_IMAGE) $(OUTPUT_IMAGE)

kernel_$(SOURCE).json : $(SOURCE).bin
	$(TRACEATLAS_ROOT)bin/newCartographer -i $< -b $(SOURCE)_autoscheduole_false_generated.bc -bi BlockInfo_$(SOURCE).json -d $(SOURCE).dot -o $@

Instance_$(SOURCE).bin : $(SOURCE).instance.native
	INSTANCE_FILE=Instance_$(SOURCE).json ./$< $(SIGMA_S) $(SIGMA_R) $(INPUT_IMAGE) $(OUTPUT_IMAGE)

# Regular rule and run
$(SOURCE).exec : $(SOURCE)_run.cpp $(SOURCE)_autoschedule_false_generated
	$(CXX) -std=c++11 -fuse-ld=lld-9 $(HALIDE_D_LINKS) $(CFLAGS) $(HALIDE_INCLUDE) $(^:%_generated=%_generated.bc) -o $@

display : $(SOURCE).exec
	./$< $(SIGMA_S) $(SIGMA_R) $(INPUT_IMAGE) $(OUTPUT_IMAGE)
	display $(INPUT_IMAGE) & display $(OUTPUT_IMAGE) &

.PHONY : clean

clean:
	rm -rf *.bc *.tr* *.native *.elf *.exec *.bin kernel_* BlockInfo_* *.dot *.ll *.a *_generated*
