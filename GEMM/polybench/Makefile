include ../../Environment.mk
SOURCE=gemm
ADDSOURCE=$(ALGORITHMS_DIR)lib/polybench/polybench.c
# 128x128 for SMALL
# 1024x1024 for STANDARD
CFLAGS += -DSTANDARD_DATASET -DPOLYBENCH_TIME
SUFFIX=.c
D_LINKS=-lm
INCLUDE=-I$(TRACEATLAS_ROOT)include/Profile/Backend/Utilities/ -I$(ALGORITHMS_DIR)lib/polybench/
POLLY_SHOW=
include ../../BuildFlow.mk
