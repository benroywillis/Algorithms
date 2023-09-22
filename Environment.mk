# set the empty variables to the installs you have on your system

# install root path for the directory, helpful with modular build files in the repo for things like program timing
ALGORITHMS_DIR=
# LLVM install to use for compilation, linking, passes, and tools. Should point to the dir with bin/ lib/ (etc)
LLVM_INSTALL=
# cyclebite install (should point to the dir with bin/ lib/ (etc)
CYCLEBITE_ROOT=
SO_PATH=$(CYCLEBITE_ROOT)lib/

# Halide install
HALIDE_INSTALL_PREFIX=
HALIDE_COMPILE_ARGS=-fno-rtti
HALIDE_D_LINKS=-lpthread -ldl -lpng -ljpeg
HALIDE_INCLUDE=-I$(HALIDE_INSTALL_PREFIX)include/ -I$(HALIDE_INSTALL_PREFIX)share/tools

# install of the dash-archives repository (only required for algorithms driven by APIs)
DASH_ROOT=
GSL_ROOT=$(DASH_ROOT)gsl/
FFTW_ROOT=$(DASH_ROOT)fftw/
OPENCV_ROOT=$(DASH_ROOT)opencv/
SRSRAN_ROOT=$(DASH_ROOT)srsran/

# Polygeist compiler
POLYGEIST_ROOT=
CGEIST=$(POLYGEIST_ROOT)bin/cgeist

# graphviz install for rendering DFGs and other Cyclebyte outputs (sudo apt install graphviz)
DOT=
# set binary to render dot files (like a web browser)
DOT_RENDER=
