LLVM_INSTALL=/mnt/heorot-10/bwilli46/LLVM9/install-release/
#LLVM_INSTALL=/mnt/heorot-10/bwilli46/LLVM12/install-release/
TRACEATLAS_ROOT=/home/bwilli46/Install/TraceAtlas_dev/
TRACEATLAS_HC_ROOT=/home/bwilli46/Install/TraceAtlas_hotCode/

# Halide install
LLVM_HALIDE_INSTALL=/mnt/heorot-10/bwilli46/LLVM12/install-release/
HALIDE_INSTALL_PREFIX=/mnt/heorot-10/bwilli46/Halide-install-release/
#HALIDE_COMPILE_ARGS=-g3 -std=c++11 -fno-rtti
HALIDE_COMPILE_ARGS=-g3 -fno-rtti
HALIDE_D_LINKS=-lpthread -ldl -lpng -ljpeg
HALIDE_INCLUDE=\
-I$(HALIDE_INSTALL_PREFIX)include/\
-I$(HALIDE_INSTALL_PREFIX)share/tools

# some library installs
GSL_ROOT=/mnt/heorot-10/bwilli46/dash-archives/debug/gsl/
FFTW_ROOT=/mnt/heorot-10/bwilli46/dash-archives/debug/fftw/

