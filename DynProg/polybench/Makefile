include ../../Environment.mk
SOURCE=dynprog
ADDSOURCE=$(ALGORITHMS_DIR)lib/polybench/polybench.c
# 128x128 for gemm
CFLAGS += -DSMALL_DATASET
SUFFIX=.c
D_LINKS=-lm
INCLUDE=-I$(TRACEATLAS_ROOT)include/Profile/Backend/Utilities/ -I$(ALGORITHMS_DIR)lib/polybench/
POLLY_SHOW=
include ../../BuildFlow.mk
