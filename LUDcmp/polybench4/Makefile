include ../../Environment.mk
SOURCE=ludcmp
ADDSOURCE=$(ALGORITHMS_DIR)lib/polybench/polybench4.2.1/polybench.c
# 128x128 for gemm
CFLAGS += -DSMALL_DATASET -DPOLYBENCH_TIME
SUFFIX=.c
D_LINKS=-lm
INCLUDE=-I$(TRACEATLAS_ROOT)include/Profile/Backend/Utilities/ -I$(ALGORITHMS_DIR)lib/polybench/polybench4.2.1/
POLLY_SHOW=
include ../../BuildFlow.mk
