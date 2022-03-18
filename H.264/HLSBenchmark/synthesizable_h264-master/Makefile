include ../../../Environment.mk
SRCDIR=tpdn_opt
SOURCE=ldecod
ADDSOURCE=$(wildcard $(SRCDIR)/*.c)
INCLUDE=-I$(TRACEATLAS_ROOT)include/ -Itpdn_opt
RARGS=input/test.264 golden/test_dec.yuv
D_LINKS=-lm
include ../../../BuildFlow.mk
