ALGORITHMS_DIR=/home/bwilli46/Algorithms/

LLVM_INSTALL=/mnt/heorot-10/bwilli46/Installs/LLVM9/release/
#LLVM_INSTALL=/mnt/heorot-10/bwilli46/Installs/LLVM12/release/
#TRACEATLAS_ROOT=/home/bwilli46/TraceAtlas/build/
#TRACEATLAS_ROOT=/home/bwilli46/TraceAtlas/build_relwithdebinfo/
TRACEATLAS_ROOT=/mnt/heorot-10/bwilli46/Installs/TraceAtlas/relwithdebinfo/
SO_PATH=LD_LIBRARY_PATH=$(TRACEATLAS_ROOT)lib/

# Halide install
# Halide 12 install
#HALIDE_INSTALL_PREFIX=/mnt/heorot-10/bwilli46/Installs/Halide-install-release/
HALIDE_INSTALL_PREFIX=/mnt/heorot-10/bwilli46/Installs/Halide10-install-release/
#HALIDE_INSTALL_PREFIX=/mnt/heorot-10/bwilli46/Installs/Halide10-install-debug/
#HALIDE_COMPILE_ARGS=-std=c++11 -fno-rtti
HALIDE_COMPILE_ARGS=-fno-rtti
HALIDE_D_LINKS=-lpthread -ldl -lpng -ljpeg
HALIDE_INCLUDE=\
-I$(HALIDE_INSTALL_PREFIX)include/\
-I$(HALIDE_INSTALL_PREFIX)share/tools

# some library installs
DASH_ROOT=/mnt/heorot-10/bwilli46/dash-archives/debug/
DASH_SOURCES_ROOT=/mnt/nobackup-09/Dash/Sources/alib/
BWILLI_ROOT=/mnt/heorot-10/bwilli46/Installs/
GSL_ROOT=$(DASH_ROOT)gsl/
FFTW_ROOT=$(DASH_ROOT)fftw/
OPENCV_ROOT=$(BWILLI_ROOT)opencv/
SRSRAN_ROOT=$(BWILLI_ROOT)srsran/debug/
