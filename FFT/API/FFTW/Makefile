include ../../../Environment.mk

SOURCE=FFT
PRECISION=0 # 0 for double, 1 for float
INPUT_DIM_N=512
INPUT_DIM_DEGREE=2
RARGS=$(INPUT_DIM_N) $(INPUT_DIM_DEGREE)

D_LINKS=-lm
DEBUG=-g3
INCLUDE=\
-I$(TRACEATLAS_ROOT)include/\
-I$(FFTW_ROOT)include/
LIBRARIES=\
$(FFTW_ROOT)lib/libfftw3.a\
$(FFTW_ROOT)lib/libfftw3f.a
CFLAGS:=-DPRECISION=$(PRECISION)
POLLY_SHOW=

include ../../../BuildFlow.mk
