# Algorithms
Just some algorithms that can be used as a reference to optimized algorithms for performance, profiling, code structuring, and other interesting things.

## TraceAtlas
The purpose of these algorithms is to test the capabilities of the TraceAtlas framework. Specifically these algorithms test how TraceAtlas can structure their code and recognize what they are doing. The build flow of this repository supports this framework. For more information about TraceAtlas, go to http://github.com/benroywillis/TraceAtlas

## Makefiles
Each Makefile in this repository facilitates the building of their respective programs using the templates BuildFlow.mk and Environment.mk. 
 * Environment.mk must be included before you use any of its variables in your Makefile
 * After overwriting compile-time and run-time flags, include BuildFlow.mk, which will take the variables you set and run them through all rules that will facilitate the analysis of $(SOURCE)

## Project Status
### GEMM
    * Naive:
    * API:
    * Halide:
### CGEMM
    * Naive:
    * API:
    * Halide:
### GaussianFilter
    * Naive:
    * API:
    * Halide:
### BilateralFilter
    * Naive:
    * API:
    * Halide:
### FFT
#### Spiral
    * Naive:
    * API:
    * Halide:
#### FFTW
    * Naive:
    * API:
    * Halide:
