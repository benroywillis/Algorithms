include ../../Environment.mk 

SOURCE=BilateralFilter
SUFFIX=.cpp
# we need to include jpeg here even though we have the static archive for it because that archive is missing a symbol "qtable"
D_LINKS=-lpthread -ldl -ljpeg $(OPENCV_ROOT)lib/opencv4/3rdparty/libippicv.a
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
$(DASH_ROOT)libjpeg/lib/libjpeg.a\
$(DASH_ROOT)libpng/lib/libpng.a\
$(DASH_ROOT)zlib/lib/libz.a

SIGMA_S=10.0
SIGMA_R=10.0
INPUT_IMAGE=../Data/john.bmp
OUTPUT_IMAGE=../Data/filtered_john.bmp
RARGS=$(SIGMA_S) $(SIGMA_R) $(INPUT_IMAGE) $(OUTPUT_IMAGE)

POLLY_SHOW=

include ../../BuildFlow.mk
