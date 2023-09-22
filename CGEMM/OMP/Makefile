include ../../Environment.mk

SOURCE=CGEMM
SUFFIX=.cpp
SIZE=64
CXXFLAGS +=-fopenmp -DSIZE=$(SIZE)
D_LINKS=-lm -lomp
RARGS=
include ../../BuildFlow.mk
