
FC=$(LLVM_ROOT)bin/flang
SOURCE=GEMM

all : run

$(SOURCE).fbin : $(SOURCE).f
	$(FC) lib.f $< -o $@

run : $(SOURCE).fbin
	./$<

.PHONY:

clean:
	rm *.fbin
