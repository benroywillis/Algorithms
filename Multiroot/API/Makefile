include ../../Environment.mk

SOURCE=multiroot

D_LINKS=-lm
INCLUDE=\
-I$(TRACEATLAS_ROOT)include/\
-I$(GSL_ROOT)include/
LIBRARIES=\
$(GSL_ROOT)lib/libgsl.a\
$(GSL_ROOT)lib/libgslcblas.a\

OPFLAG=-O0
DEBUG=-g3

include ../../BuildFlow.mk
