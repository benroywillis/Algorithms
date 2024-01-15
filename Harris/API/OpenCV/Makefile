include ../../../Environment.mk 

#ifeq ($(OPFLAG), -O0)
#	OPENCV_ROOT=$(DASH_ROOT)../debug/opencv/
#else
#	OPENCV_ROOT=$(DASH_ROOT)../release/opencv/
#endif

SOURCE=Harris
SUFFIX=.cpp
# we need to include jpeg here even though we have the static archive for it because that archive is missing a symbol "qtable"
D_LINKS=-lpthread -ldl $(OPENCV_ROOT)lib/opencv4/3rdparty/libippicv.a
INCLUDE=-I$(OPENCV_ROOT)include/opencv4/
LIBRARIES=\
$(OPENCV_ROOT)lib/libopencv_imgproc.a\
$(OPENCV_ROOT)lib/libopencv_ximgproc.a\
$(OPENCV_ROOT)lib/libopencv_imgcodecs.a\
$(OPENCV_ROOT)lib/libopencv_core.a\
$(OPENCV_ROOT)lib/opencv4/3rdparty/libittnotify.a\
$(OPENCV_ROOT)lib/opencv4/3rdparty/libippiw.a\
$(OPENCV_ROOT)lib/opencv4/3rdparty/liblibtiff.a\
$(OPENCV_ROOT)lib/opencv4/3rdparty/liblibwebp.a\
$(OPENCV_ROOT)lib/opencv4/3rdparty/liblibopenjp2.a\
$(OPENCV_ROOT)lib/opencv4/3rdparty/libIlmImf.a\
$(OPENCV_ROOT)lib/opencv4/3rdparty/liblibjpeg-turbo.a\
$(OPENCV_ROOT)lib/opencv4/3rdparty/liblibopenjp2.a\
$(OPENCV_ROOT)lib/opencv4/3rdparty/liblibpng.a\
$(OPENCV_ROOT)lib/opencv4/3rdparty/libzlib.a\
$(OPENCV_ROOT)lib/opencv4/3rdparty/libtbb.a
INPUT_IMAGE=../../../Data/Input/sample_1920_1280.bmp
OUTPUT_IMAGE=../../../Data/Output/Harris_OpenCV_sample_1920_1280.bmp
NUM_THREADS?=1
RARGS=$(INPUT_IMAGE) $(OUTPUT_IMAGE) $(NUM_THREADS)

POLLY_SHOW=

include ../../../BuildFlow.mk
