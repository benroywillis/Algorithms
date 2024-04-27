include ../../Environment.mk
SOURCE=Hist
SUFFIX=.cpp
D_LINKS=-lm
INCLUDE=-I$(TRACEATLAS_ROOT)include/Profile/Backend/Utilities/ -I../../TimingLib/
POLLY_SHOW=
SIZE?=512
# 0 -> uint32_t, 1 -> uint16_t, anything else -> uint8_t
PRECISION?=0
CXXFLAGS += -DSIZE=$(SIZE) -DPRECISION=$(PRECISION)
include ../../BuildFlow.mk
