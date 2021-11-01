include ../../Environment.mk

PERFECT_LIB_PATH=../../lib/PERFECT/
SOURCE=main
SUFFIX=.c
ADDSOURCE=sort$(SUFFIX) quicksort$(SUFFIX) validate$(SUFFIX) $(PERFECT_LIB_PATH)xmem/xmalloc$(SUFFIX) $(PERFECT_LIB_PATH)timing/timer$(SUFFIX)
# the default is quicksort (by defining none of the below macros defined)
# USE_RADIX_SORT USE_N_SQUARED_SORT USE_INSERTION_SORT
CFLAGS+=-DUSE_INSERTION_SORT

INCLUDE=-I$(TRACEATLAS_ROOT)include/ -I. -I$(PERFECT_LIB_PATH)
POLLY_SHOW=

include ../../BuildFlow.mk
