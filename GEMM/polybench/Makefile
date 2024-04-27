include ../../Environment.mk
SOURCE=gemm_nonstatic
ADDSOURCE=$(ALGORITHMS_DIR)lib/polybench/polybench.c
# 128x128 for SMALL
# 1024x1024 for STANDARD
CFLAGS += -DSTANDARD_DATASET -DPOLYBENCH_TIME
#CFLAGS += -DSMALL_DATASET -DPOLYBENCH_TIME
SUFFIX=.c
D_LINKS=-lm
INCLUDE=-I$(TRACEATLAS_ROOT)include/Profile/Backend/Utilities/ -I$(ALGORITHMS_DIR)lib/polybench/
POLLY_SHOW=
# array_idx_min NI NK NJ
SIZE?=512
RARGS= 0 $(SIZE) $(SIZE) $(SIZE)
include ../../BuildFlow.mk
