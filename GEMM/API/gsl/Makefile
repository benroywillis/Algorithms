
SOURCE=gsl_GEMM
MATRIX_DIM_A=512
MATRIX_DIM_B=512
MATRIX_DIM_C=512
ALPHA=1.0
BETA=1.0
TRANSPOSE=0
RARGS=$(MATRIX_DIM_A) $(MATRIX_DIM_B) $(MATRIX_DIM_C) $(ALPHA) $(BETA) $(TRANSPOSE)

include ../../../Environment.mk
D_LINKS=
INCLUDE=\
-I$(TRACEATLAS_ROOT)include/\
-I$(GSL_ROOT)include/
LIBRARIES=\
$(GSL_ROOT)lib/libgsl.a\
$(GSL_ROOT)lib/libgslcblas.a

# turn off scop rendering
POLLY_SHOW=
include ../../../BuildFlow.mk
