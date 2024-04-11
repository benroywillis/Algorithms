include ../../Environment.mk
SOURCE=bfs
ADDSOURCE=graph_ge.c
SUFFIX=.cpp
D_LINKS=-lm
POLLY_SHOW=
# 0 for double, 1 for float, 2 for long, 3 or more for int
PRECISION?=3
# size is the number of vertices in the graph and edges/3
SIZE?=128
CXXFLAGS += -DSIZE=$(SIZE) -DPRECISION=$(PRECISION) -Wno-writable-strings
include ../../BuildFlow.mk
