include ../../Environment.mk
SOURCE=relu
SUFFIX=.cpp
D_LINKS=-lm
POLLY_SHOW=
ROWS?=64
COLS?=64
PRECISION?=0
CXXFLAGS += -DROWS=$(ROWS) -DCOLS=$(COLS) -DPRECISION=$(PRECISION)
include ../../BuildFlow.mk
