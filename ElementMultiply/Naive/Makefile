include ../../Environment.mk
SOURCE=ElementMultiply
SUFFIX=.cpp
INCLUDE=-I$(TRACEATLAS_ROOT)include/Profile/Backend/Utilities/ -I../../TimingLib/
POLLY_SHOW=
SIZE?=512
CXXFLAGS += -DSIZE=$(SIZE) -DPRINT_TIMES
include ../../BuildFlow.mk
