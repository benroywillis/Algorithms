include ../../../Environment.mk 

#LLVM_INSTALL=/mnt/heorot-10/bwilli46/Installs/LLVM13/release/

ifeq ($(OPFLAG), -O0)
	EIGEN_PATH=$(DASH_ROOT)../debug/eigen/include/eigen3/
	LIBPNG_PATH=$(DASH_ROOT)../debug/libpng/
	LIBJPEG_PATH=$(DASH_ROOT)../debug/libjpeg/
	OPENCV_PATH=$(DASH_ROOT)../debug/opencv/
else
	EIGEN_PATH=$(DASH_ROOT)../release/eigen/include/eigen3/
	LIBPNG_PATH=$(DASH_ROOT)../release/libpng/
	LIBJPEG_PATH=$(DASH_ROOT)../release/libjpeg/
	OPENCV_PATH=$(DASH_ROOT)../release/opencv/
endif

SOURCE=Harris
SUFFIX=.cpp
# we need to include jpeg here even though we have the static archive for it because that archive is missing a symbol "qtable"
D_LINKS=
INCLUDE=-I$(EIGEN_PATH) -I$(LIBJPEG_PATH)include/ -I$(LIBPNG_PATH)include/ -Ilibnvision/include/
LIBRARIES=\
$(LIBJPEG_PATH)lib/libjpeg.a\
$(LIBPNG_PATH)lib/libpng.a

# if you use the latter input for this application it has a bad alloc (likely caused by static array somewhere)
INPUT_IMAGE=../../../Data/Input/CheckerBoard.bmp
#INPUT_IMAGE=../../../Data/Input/sample_1920_1280.bmp
OUTPUT_IMAGE=../../../Data/Output/CheckerBoard.bmp
#OUTPUT_IMAGE=../../../Data/Output/Harris_OpenCV_sample_1920_1280.bmp
RARGS=$(INPUT_IMAGE) $(OUTPUT_IMAGE)

POLLY_SHOW=

include ../../../BuildFlow.mk
