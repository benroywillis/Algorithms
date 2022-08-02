# Algorithms
Just some algorithms that can be used as a reference to optimized algorithms for performance, profiling, code structuring, and other interesting things.

## TraceAtlas
The purpose of these algorithms is to test the capabilities of the TraceAtlas framework. Specifically these algorithms test how TraceAtlas can structure their code and recognize what they are doing. The build flow of this repository supports this framework. For more information about TraceAtlas, go to http://github.com/benroywillis/TraceAtlas

## Makefiles
Each Makefile in this repository facilitates the building of their respective programs using the templates BuildFlow.mk and Environment.mk. 
 * Environment.mk must be included before you use any of its variables in your Makefile
 * After overwriting compile-time and run-time flags, include BuildFlow.mk, which will take the variables you set and run them through all rules that will facilitate the analysis of $(SOURCE)

## Project Status
### 2DConvolution
	* Naive: 0%
	* API: 0%
	* DSLs: Halide - done
	* Benchmarks: PERFECT - done
### BilateralFilter
    * Naive: done
    * API: OpenCV - done
    * DSLs: Halide - needs scheduling
### BilateralGrid
	* Naive: 20%
	* API: OpenCV - done
	* DSLs: Halide - done
	* Benchmarks: PERFECT - 0%
## Canny Edge Detection
	* Naive: 0%
	* API: OpenCV - done
	* DSLs: Halide - 0%
### CGEMM
    * Naive: done
### DWT
	* Naive: 0%
	* API: 0%
	* DSLs: Halide - 50%
	* Benchmarks: PERFECT - done
### FFT
	* Naive: done (recursive)
	* API: done (FFTW)
	* DSLs: Halide - done 
	* Benchmarks: PERFECT - done
### GaussianFilter
    * Naive: done
    * API: OpenCV - 0%
    * DSLs: Halide - needs scheduling
### GEMM
    * Naive: done
    * API: done
    * DSLs: Halide - done
    * Optimized: done
### H.264
	* AccelSeeker: done
### HistEq
	* DSLs: Halide - 50%
	* Benchmarks: PERFECT - done
### HoughTransform (line)
	* Naive: 0%
	* DSLs: Halide - 100%
### RayTracer
	* API: RTInOneWeekend - done
### SAR
	* DSLs: Halide - 50%
	* Benchmarks: PERFECT - done
### Sort
	* DSLs: Halide - 50%
	* Benchmarks: PERFECT - done
### STAP
	* DSLs: Halide - 50%
	* Benchmarks: PERFECT - done
### SVD
	* API: GSL - done
### WAMI
	* DSLs: Halide - 50%
	* Benchmarks: PERFECT - done
