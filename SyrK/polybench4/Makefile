include ../../Environment.mk
SOURCE=syrk
ADDSOURCE=$(ALGORITHMS_DIR)lib/polybench/polybench4.2.1/polybench.c
# 128 for small, 1024 for standard
CFLAGS += -DSMALL_DATASET -DPOLYBENCH_TIME
#CFLAGS += -DSTANDARD_DATASET -DPOLYBENCH_TIME
SUFFIX=.c
D_LINKS=-lm
INCLUDE=-I$(TRACEATLAS_ROOT)include/Profile/Backend/Utilities/ -I$(ALGORITHMS_DIR)lib/polybench/polybench4.2.1/
POLLY_SHOW=
include ../../BuildFlow.mk
