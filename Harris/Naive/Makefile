include ../../Environment.mk

SOURCE=Harris
SUFFIX=.cpp
ADDSOURCE=../../BilateralFilter/Naive/BilateralFilter.c

# compile time args
PRECISION?=float
CFLAGS += -DPRECISION=$(PRECISION)

ifeq ($(OPFLAG), -O0)
	OPENCV_PATH=$(DASH_ROOT)../debug/opencv/
else
	OPENCV_PATH=$(DASH_ROOT)../release/opencv/
endif

# opencv dependencies
INCLUDE=-I$(OPENCV_PATH)include/opencv4/ -I$(TRACEATLAS_ROOT)include/ -I../../BilateralFilter/Naive/
D_LINKS=-lm -lpthread -ldl
LIBRARIES=\
$(OPENCV_PATH)lib/libopencv_imgproc.a\
$(OPENCV_PATH)lib/libopencv_ximgproc.a\
$(OPENCV_PATH)lib/libopencv_imgcodecs.a\
$(OPENCV_PATH)lib/libopencv_core.a\
$(OPENCV_PATH)lib/opencv4/3rdparty/libittnotify.a\
$(OPENCV_PATH)lib/opencv4/3rdparty/libippiw.a\
$(OPENCV_PATH)lib/opencv4/3rdparty/libippicv.a\
$(OPENCV_PATH)lib/opencv4/3rdparty/liblibtiff.a\
$(OPENCV_PATH)lib/opencv4/3rdparty/liblibwebp.a\
$(OPENCV_PATH)lib/opencv4/3rdparty/liblibopenjp2.a\
$(OPENCV_PATH)lib/opencv4/3rdparty/libIlmImf.a\
$(OPENCV_PATH)lib/opencv4/3rdparty/liblibjpeg-turbo.a\
$(OPENCV_PATH)lib/opencv4/3rdparty/liblibopenjp2.a\
$(OPENCV_PATH)lib/opencv4/3rdparty/liblibpng.a\
$(OPENCV_PATH)lib/opencv4/3rdparty/libzlib.a\
$(OPENCV_PATH)lib/opencv4/3rdparty/libtbb.a

# runtime args
INPUT_IMAGE=../../Data/Input/sample_1920_1280.bmp
OUTPUT_IMAGE=../../Data/Output/Harris_Naive_sample_1920_1280.bmp
RARGS=$(ALPHA) $(INPUT_IMAGE) $(OUTPUT_IMAGE)

POLLY_SHOW=
include ../../BuildFlow.mk
