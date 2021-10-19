include ../../../Environment.mk

SOURCE=FFT
INPUT_DIM_N=512
RARGS=$(INPUT_DIM_N)

D_LINKS=-lm
DEBUG=-g3
INCLUDE=\
-I$(TRACEATLAS_ROOT)include/\
-I$(FFTW_ROOT)include/
LIBRARIES=\
$(FFTW_ROOT)lib/libfftw3.a\
$(FFTW_ROOT)lib/libfftw3f.a
POLLY_SHOW=

include ../../../BuildFlow.mk
