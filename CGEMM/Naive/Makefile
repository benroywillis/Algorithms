include ../../Environment.mk

SOURCE=CGEMM
SUFFIX=.cpp
SIZE=64
CXXFLAGS += -DSIZE=$(SIZE)
D_LINKS=-lm -lomp
CXXFLAGS += -fopenmp -Rpass=openmp-opt -Rpass-analysis=openmp-opt -Rpass-missed=openmp-opt

include ../../BuildFlow.mk
