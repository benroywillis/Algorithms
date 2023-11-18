include ../../../Environment.mk

CFLAGS?=-Wall -Wno-unused-label

SOURCE=gemm
ADDSOURCE=local_support.c ../../common/support.c ../../common/harness.c
INCLUDE=-I../../common/
RARGS=input.input_data check.input_data

#SRCS=$(KERN).c local_support.c ../../common/support.c
#FILES=$(SRCS) $(KERN).h ../../common/support.h

#$(KERN): $(FILES) ../../common/harness.c
#	$(CC) $(CFLAGS) -I../../common -o $(KERN) $(SRCS) ../../common/harness.c

#run: $(KERN) input.data check.data
#	./$(KERN) input.data check.data

#generate: $(FILES) generate.c
#	$(CC) $(CFLAGS) -I../../common -o generate $(SRCS) generate.c
#	./generate

include ../../../BuildFlow.mk
