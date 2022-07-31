# This project runs a HoughLine transform

include ../../Environment.mk

SOURCE=HoughTransform
INPUT_IMAGE=../../BilateralFilter/Data/john.pnm
OUTPUT_IMAGE=../../BilateralFilter/Data/filtered_john.pnm
RARGS=$(INPUT_IMAGE) $(OUTPUT_IMAGE)

OPFLAG=-O3
DEBUG=-g0
POLLY_SHOW=
include ../../BuildFlow.mk
