include ../../../Environment.mk

SPIRAL_ROOT=/mnt/heorot-10/bwilli46/spiral-software/

PRECISION=0 # 0 for double, 1 for float
INPUT_DIM_DEGREE=2
ifeq ($(INPUT_DIM_DEGREE), 2)
SOURCE=MDDFT
else
SOURCE=DFT
endif

D_LINKS=-lm
INCLUDE=\
-I$(SPIRAL_ROOT)namespaces/
CFLAGS:=-DPRECISION=$(PRECISION)

include ../../../BuildFlow.mk
