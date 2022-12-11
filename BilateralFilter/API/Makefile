include ../../Environment.mk 

SOURCE=BilateralFilter
SUFFIX=.cpp

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
$(OPENCV_ROOT)lib/opencv4/3rdparty/libzlib.a\
$(OPENCV_ROOT)lib/opencv4/3rdparty/libtbb.a

# for building the project with the old install of OpenCV
#DASH_ROOT=/mnt/nobackup-09/Dash/Sources/

#OPENCV_DIR=$(DASH_ROOT)alib/

#LIBRARIES = \
$(OPENCV_DIR)libopencv_imgproc.a\
$(OPENCV_DIR)libopencv_videoio.a\
$(OPENCV_DIR)libopencv_dnn.a\
$(OPENCV_DIR)libopencv_stitching.a\
$(OPENCV_DIR)libopencv_flann.a\
$(OPENCV_DIR)libopencv_core.a\
$(OPENCV_DIR)libopencv_calib3d.a\
$(OPENCV_DIR)libopencv_photo.a\
$(OPENCV_DIR)libopencv_imgcodecs.a\
$(OPENCV_DIR)libopencv_video.a\
$(OPENCV_DIR)libopencv_highgui.a\
$(OPENCV_DIR)libopencv_objdetect.a\
$(OPENCV_DIR)libopencv_ml.a\
$(OPENCV_DIR)libopencv_features2d.a\
$(OPENCV_DIR)libquirc.a\
$(OPENCV_DIR)liblibwebp.a\
$(OPENCV_DIR)liblibpng.a\
$(OPENCV_DIR)libIlmImf.a\
$(OPENCV_DIR)liblibjasper.a\
$(OPENCV_DIR)liblibprotobuf.a\
$(OPENCV_DIR)liblibtiff.a\
$(OPENCV_DIR)liblibjpeg-turbo.a\
$(OPENCV_DIR)libittnotify.a\

#INCLUDE=-I$(DASH_ROOT)include/opencv4/ -I$(DASH_ROOT)/include/
#D_LINKS=-lz -lpng -ljpeg -ldl -lrt -lpthread
#OPFLAG=-O0

SIGMA_S=10.0
SIGMA_R=10.0
INPUT_IMAGE=../../Data/Input/john.bmp
OUTPUT_IMAGE=../../Data/Output/OpenCV_BilateralFilter_john.bmp
RARGS=$(SIGMA_S) $(SIGMA_R) $(INPUT_IMAGE) $(OUTPUT_IMAGE)

POLLY_SHOW=

include ../../BuildFlow.mk
