include ../../Environment.mk

SOURCE=BilateralFilter
SIGMA_S=1.0
SIGMA_R=1.0
INPUT_IMAGE=john.bmp
OUTPUT_IMAGE=filtered_john.bmp
RARGS=$(SIGMA_S) $(SIGMA_R) $(INPUT_IMAGE) $(OUTPUT_IMAGE)
D_LINKS=-lm
INCLUDE=-I$(TRACEATLAS_ROOT)include/

include ../../BuildFlow.mk
