include ../../Environment.mk 

SOURCE=HoughTransform
SUFFIX=.cpp
# we need to include jpeg here even though we have the static archive for it because that archive is missing a symbol "qtable"

# old OpenCV build (v4.3.0)
#OPENCV_ROOT=$(DASH_SOURCES_ROOT)
#INCLUDE=-I$(OPENCV_ROOT)../include/opencv4/
#D_LINKS=-lpthread -ldl -lz -ljpeg
#LIBRARIES=\
$(OPENCV_ROOT)/libopencv_imgproc.a\
$(OPENCV_ROOT)/libopencv_imgcodecs.a\
$(OPENCV_ROOT)/libopencv_core.a\
$(OPENCV_ROOT)/libittnotify.a\
$(OPENCV_ROOT)/liblibtiff.a\
$(OPENCV_ROOT)/liblibwebp.a\
$(OPENCV_ROOT)/libIlmImf.a\
$(OPENCV_ROOT)/liblibjpeg-turbo.a\
$(OPENCV_ROOT)/liblibpng.a\
$(OPENCV_ROOT)/liblibjasper.a

# new OpenCV build (v4.6.0)
OPENCV_ROOT=$(BWILLI_ROOT)/opencv/debug/
D_LINKS=-lpthread -ldl $(OPENCV_ROOT)lib/opencv4/3rdparty/libippicv.a
INCLUDE=-I$(OPENCV_ROOT)include/opencv4/
LIBRARIES=\
$(OPENCV_ROOT)lib/libopencv_imgproc.a\
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
$(OPENCV_ROOT)lib/opencv4/3rdparty/libzlib.a

OPFLAG=-O0

# Inverse ratio of accumulator resolution to the input image resolution.
DP=1
# Minimum distance (in pixels) between centers of circles
MINDIST=16
INPUT_IMAGE=../../BilateralFilter/Data/john.bmp
OUTPUT_IMAGE=../../BilateralFilter/Data/houghCircles_john.bmp
RARGS=$(DP) $(MINDIST) $(INPUT_IMAGE) $(OUTPUT_IMAGE)

POLLY_SHOW=

include ../../BuildFlow.mk
