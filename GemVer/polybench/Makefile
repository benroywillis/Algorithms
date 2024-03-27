include ../../Environment.mk
SOURCE=gemver
ADDSOURCE=$(ALGORITHMS_DIR)lib/polybench/polybench.c
# 500 for small
# 4096 for standard
CFLAGS += -DSTANDARD_DATASET -DPOLYBENCH_TIME
SUFFIX=.c
D_LINKS=-lm
INCLUDE=-I$(TRACEATLAS_ROOT)include/Profile/Backend/Utilities/ -I$(ALGORITHMS_DIR)lib/polybench/
POLLY_SHOW=
include ../../BuildFlow.mk
