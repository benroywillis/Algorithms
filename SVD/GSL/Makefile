include ../../Environment.mk

SOURCE=gsl_svd
RARGS=512 256
D_LINKS=-lm
INCLUDE=\
-I$(TRACEATLAS_ROOT)include/\
-I$(GSL_ROOT)include/
LIBRARIES=\
$(GSL_ROOT)lib/libgsl.a\
$(GSL_ROOT)lib/libgslcblas.a
# turn off scop rendering
POLLY_SHOW=

include ../../BuildFlow.mk
