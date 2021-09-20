
CC?=clang-9
CXX?=clang++-9
OPT?=opt-9
LDFLAGS?=-flto -fuse-ld=lld-9 -Wl,--plugin-opt=emit-llvm -g3 -O0

SOURCE=BilateralFilter
# we need to include jpeg here even though we have the static archive for it because that archive is missing a symbol "qtable"
D_LINKS=-lpthread -ldl -ljpeg
DEBUG=
#TRACEATLAS_ROOT=/home/bwilli46/TraceAtlas/build2/
TRACEATLAS_ROOT=/home/bwilli46/Install/TraceAtlas/build2/
DASH_ROOT=/mnt/heorot-10/bwilli46/dash-archives/
OPENCV_ROOT=$(DASH_ROOT)debug/opencv/
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
	$(CC) $(LDFLAGS) $(INCLUDE) $(LIBRARIES) $< -o $@

$(SOURCE).markov.bc: $(SOURCE).bc
	$(OPT) -load $(TRACEATLAS_ROOT)/lib/AtlasPasses.so -Markov $< -o $@

$(SOURCE).instance.bc : $(SOURCE).bc
	$(OPT) -load $(TRACEATLAS_ROOT)/lib/AtlasPasses.so -Instance $< -o $@

$(SOURCE).markov.native : $(SOURCE).markov.bc
	$(CXX) -fuse-ld=lld-9 $(D_LINKS) $(OPENCV_ROOT)lib/opencv4/3rdparty/libippicv.a $(TRACEATLAS_ROOT)/lib/libAtlasBackend.a $< -o $@

$(SOURCE).instance.native : $(SOURCE).instance.bc
	$(CXX) -fuse-ld=lld-9 $(D_LINKS) $(TRACEATLAS_ROOT)/lib/libAtlasBackend.a $< -o $@

$(SOURCE).bin : $(SOURCE).markov.native
	BLOCK_FILE=BlockInfo_$(SOURCE).json MARKOV_FILE=$(SOURCE).bin ./$< output.image john.ppm $(SIGMA_S) $(SIGMA_R)

kernel_$(SOURCE).json : $(SOURCE).bin
	$(TRACEATLAS_ROOT)/bin/newCartographer -i $< -b $(SOURCE).bc -bi BlockInfo_$(SOURCE).json -d dot_$(SOURCE).dot -o $@

Instance_$(SOURCE).json : $(SOURCE).instance.native kernel_$(SOURCE).json
	KERNEL_FILE=kernel_$(SOURCE).json INSTANCE_FILE=$@ ./$< output.image john.ppm $(SIGMA_S) $(SIGMA_R)

# just builds the source code into elf form
elf : $(SOURCE).cpp
	$(CXX) -fuse-ld=lld-9 $(D_LINKS) -g3 -O0 $(DEBUG) $(INCLUDE) $(LIBRARIES) $< -o $(SOURCE).elf

display : elf
	./$(SOURCE).elf $(SIGMA_S) $(SIGMA_R) $(INPUT_IMAGE) $(OUTPUT_IMAGE)
	display $(INPUT_IMAGE) & display $(OUTPUT_IMAGE)

clean:
	rm -rf *.bc *.ll *.tr* *.bin *.json *.exec *.elf *.native *.dot
