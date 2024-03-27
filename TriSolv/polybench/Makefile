include ../../Environment.mk
SOURCE=trisolv
ADDSOURCE=$(ALGORITHMS_DIR)lib/polybench/polybench.c
# 500 for small, 4000 for standard
CFLAGS += -DSTANDARD_DATASET
SUFFIX=.c
D_LINKS=-lm
INCLUDE=-I$(TRACEATLAS_ROOT)include/Profile/Backend/Utilities/ -I$(ALGORITHMS_DIR)lib/polybench/
POLLY_SHOW=
include ../../BuildFlow.mk
