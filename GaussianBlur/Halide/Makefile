include ../../Environment.mk

# LLVM12 install
LLVM_INSTALL_PREFIX=/mnt/heorot-10/bwilli46/LLVM12/install-release/
HALIDE=1
HALIDE_AUTOSCHEDULE=0
# In order for this variable to work, your run files need to be named
# $(SOURCE)_generate.cpp $(SOURCE)_run.cpp
# and your generator (-g <generator_name>) needs to match this variable
SOURCE=gaussian_filter
SUFFIX=.cpp
OPT_TEST=Instance

SIGMA_S=1.0
SIGMA_R=1.0
INPUT_IMAGE=john.png
OUTPUT_IMAGE=filteredjohn.png

AUTOSCHEDULE=0

include ../../BuildFlow.mk
