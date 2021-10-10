
LLVM_ROOT=/usr/
#TRACEATLAS_ROOT=/home/bwilli46/TraceAtlas/build2/
TRACEATLAS_ROOT=/home/bwilli46/Install/TraceAtlas_dev/
TRACEATLAS_HC_ROOT=/home/bwilli46/Install/TraceAtlas_hotCode/
DASH_ROOT=/mnt/heorot-10/bwilli46/dash-archives/
OPENCV_ROOT=$(DASH_ROOT)debug/opencv/

CC?=clang-9
CXX?=clang++-9
OPT?=opt-9
LLD?=-fuse-ld=$(LLVM_ROOT)bin/ld.lld-9
LDFLAGS?=-flto $(LLD) -Wl,--plugin-opt=emit-llvm

GCC=gcc
GXX=g++
GLD=ld

OPFLAG=-O0
DEBUG=-g0

SOURCE=BilateralFilter
# we need to include jpeg here even though we have the static archive for it because that archive is missing a symbol "qtable"
D_LINKS=-lpthread -ldl -ljpeg
INCLUDE=-I$(OPENCV_ROOT)include/opencv4/
LIBRARIES=\
$(OPENCV_ROOT)lib/libopencv_imgproc.a\
$(OPENCV_ROOT)lib/libopencv_imgcodecs.a\
$(OPENCV_ROOT)lib/libopencv_core.a\
$(OPENCV_ROOT)lib/opencv4/3rdparty/libittnotify.a\
$(OPENCV_ROOT)lib/opencv4/3rdparty/libippicv.a\
$(OPENCV_ROOT)lib/opencv4/3rdparty/libippiw.a\
$(OPENCV_ROOT)lib/opencv4/3rdparty/liblibtiff.a\
$(OPENCV_ROOT)lib/opencv4/3rdparty/liblibwebp.a\
$(OPENCV_ROOT)lib/opencv4/3rdparty/liblibopenjp2.a\
$(OPENCV_ROOT)lib/opencv4/3rdparty/libIlmImf.a\
$(DASH_ROOT)debug/libjpeg/lib/libjpeg.a\
$(DASH_ROOT)debug/libpng/lib/libpng.a\
$(DASH_ROOT)debug/zlib/lib/libz.a

SIGMA_S=10.0
SIGMA_R=10.0
INPUT_IMAGE=john.bmp
OUTPUT_IMAGE=filtered_john.bmp

all: Instance_$(SOURCE).json

$(SOURCE).bc : $(SOURCE).cpp
	$(CXX) $(OPFLAG) $(DEBUG) $(LDFLAGS) $(INCLUDE) $(LIBRARIES) $< -o $@

$(SOURCE).markov.bc: $(SOURCE).bc
	$(OPT) -load $(TRACEATLAS_ROOT)/lib/AtlasPasses.so -Markov $< -o $@

$(SOURCE).hotcode.bc: $(SOURCE).bc
	$(OPT) -load $(TRACEATLAS_HC_ROOT)/lib/AtlasPasses.so -Markov $< -o $@

$(SOURCE).instance.bc : $(SOURCE).bc
	$(OPT) -load $(TRACEATLAS_ROOT)/lib/AtlasPasses.so -Instance $< -o $@

$(SOURCE).markov.native : $(SOURCE).markov.bc
	$(CXX) $(OPFLAG) $(DEBUG) $(LLD) $(D_LINKS) $(OPENCV_ROOT)lib/opencv4/3rdparty/libippicv.a $(TRACEATLAS_ROOT)/lib/libAtlasBackend.a $< -o $@

$(SOURCE).instance.native : $(SOURCE).instance.bc
	$(CXX) $(OPFLAG) $(DEBUG) $(LLD) $(D_LINKS) $(OPENCV_ROOT)lib/opencv4/3rdparty/libippicv.a $(TRACEATLAS_ROOT)/lib/libAtlasBackend.a $< -o $@

$(SOURCE).hotcode.native : $(SOURCE).hotcode.bc
	$(CXX) $(OPFLAG) $(DEBUG) $(LLD) $(D_LINKS) $(OPENCV_ROOT)lib/opencv4/3rdparty/libippicv.a $(TRACEATLAS_HC_ROOT)/lib/libAtlasBackend.a $< -o $@

$(SOURCE).bin : $(SOURCE).markov.native
	BLOCK_FILE=BlockInfo_$(SOURCE).json MARKOV_FILE=$(SOURCE).bin ./$< $(SIGMA_S) $(SIGMA_R) $(INPUT_IMAGE) $(OUTPUT_IMAGE)

$(SOURCE).hotcode.bin : $(SOURCE).hotcode.native
	BLOCK_FILE=BlockInfo_$(SOURCE).hotcode.json MARKOV_FILE=$@ ./$< $(SIGMA_S) $(SIGMA_R) $(INPUT_IMAGE) $(OUTPUT_IMAGE)

Instance_$(SOURCE).json : $(SOURCE).instance.native kernel_$(SOURCE).json
	KERNEL_FILE=kernel_$(SOURCE).json INSTANCE_FILE=$@ ./$< $(SIGMA_S) $(SIGMA_R) $(INPUT_IMAGE) $(OUTPUT_IMAGE)

kernel_$(SOURCE).json : $(SOURCE).bin
	$(TRACEATLAS_ROOT)/bin/newCartographer -i $< -b $(SOURCE).bc -bi BlockInfo_$(SOURCE).json -d dot_$(SOURCE).dot -o $@

kernel_$(SOURCE).hotcode.json : $(SOURCE).hotcode.bin
	$(TRACEATLAS_HC_ROOT)/bin/newCartographer -h -i $< -b $(SOURCE).bc -bi BlockInfo_$(SOURCE).hotcode.json -d dot_$(SOURCE).hotcode.dot -o $@

# tik with polly
tik_polly_$(SOURCE).bc : tik_$(SOURCE).bc
	$(CXX) $(OPFLAG) $(DEBUG) $(LDFLAGS) $(POLLYFLAGS) -S $< -o $@

ts_polly_$(SOURCE).bc : tik_polly_$(SOURCE).bc $(SOURCE).bc
	$(TRACEATLAS_ROOT)bin/tikSwap -S -t $< -b $(SOURCE).bc -o $@

ts_polly_$(SOURCE).exec : ts_polly_$(SOURCE).bc tik_polly_$(SOURCE).bc
	$(CXX) $(OPFLAG) $(DEBUG) $^ -o $@

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
elf : $(SOURCE).cpp
	$(CXX) $(OPFLAG) $(DEBUG) $(LLD) $(INCLUDE) $(D_LINKS) $(LIBRARIES) $< -o $(SOURCE).elf

run : elf
	./$(SOURCE).elf $(SIGMA_S) $(SIGMA_R) $(INPUT_IMAGE) $(OUTPUT_IMAGE)

display : run
	display $(INPUT_IMAGE) &
	display $(OUTPUT_IMAGE) &

gprof_$(SOURCE).elf : $(SOURCE).cpp
	$(GXX) $(INCLUDE) $(OPFLAG) $(DEBUG) -c -pg -Wno-unused-result $< -o gprof_$(SOURCE).obj
	$(GXX) -pg gprof_$(SOURCE).obj $(D_LINKS) -o $@
	./$@ $(SIGMA_S) $(SIGMA_R) $(INPUT_IMAGE) $(OUTPUT_IMAGE) 

gprof : gprof_$(SOURCE).elf
	gprof -l ./$<

gcov_$(SOURCE).elf : $(SOURCE).cpp
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

elf_polly : $(SOURCE).cpp
	$(CXX) $(OPFLAG) $(DEBUG) $(INCLUDE) $(D_LINKS) -mllvm -polly -mllvm -polly-show-only -mllvm -polly-allow-nonaffine -mllvm -polly-allow-nonaffine-branches -mllvm -polly-allow-nonaffine-loops $< -o $(SOURCE).elf_polly

.PHONY:

clean:
	rm -rf *.bc *.ll *.tr* *.bin *.json *.exec *.elf* *.native *.dot *.bc.h *.elf *.obj *.gcda *.gcno *.gcov *.log *.data *.out

clean_oprofile:
	sudo rm -rf oprofile_data
