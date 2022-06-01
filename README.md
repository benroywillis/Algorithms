# Algorithms
A small corpus of programs that verify, compare and hand-compile the structuring results of TraceAtlas and state-of-the-art tools.

## TraceAtlas
The purpose of these algorithms is to test the capabilities of the TraceAtlas framework. Specifically these algorithms test how TraceAtlas can structure their code and recognize what they are doing. The build flow of this repository supports this framework. For more information about TraceAtlas, go to http://github.com/benroywillis/TraceAtlas

## Makefiles
Each Makefile in this repository facilitates the building of their respective programs using the templates BuildFlow.mk and Environment.mk.

### Environment.mk
Environment.mk contains variables that point to installations of the compiler toolchain, external dependencies and other configurations. See the example on where this file should be included in your Makefile.

### Buildflow.mk
After overwriting compile-time and run-time flags, include BuildFlow.mk, which will take the variables you set and run them through all rules that will facilitate the analysis of $(SOURCE). See the example for variables that should be set to facilitate the buildflow.

## Example
The following Makefile example uses Buildflow.mk and Environment.mk to facilitate the TraceAtlas pipeline for a custom project with input program file `heapsort.c`. It includes a include directory paths (INCLUDE), external dependencies (dynamic links are included with D_LINKS, these libraries are not profiled by TraceAtlas) (static bitcode archives that are profiled by TraceAtlas are included with LIBRARIES), additional source files (ADDSOURCE), and runtime args (RARGS).

```
# should define all variables that point to installations and custom configurations. Included here so all variables are accessible when defining variables below
include Environment.mk

# this is the source file that contains "main", all resulting files will be named after this one
SOURCE=heapsort
# this parameter specifies what the source file type is, which is this case is a C source file (which instructs which compiler should be used in Buildflow.mk. By default this variable is set to ".c", and if it is not ".c" by the time Buildflow.mk is included, the c++ compiler will be used.)
SUFFIX=.c
# this is an additional source file whose symbols are used by heapsort
ADDSOURCE=heap_ops
# this arg specifies shared objects that need to be linked in order for the resulting program to work. None of the symbols from this dynamic library will be profiled
D_LINKS=-lm
# this path points to headers that need to be included for the API that is used in the program
INCLUDE=\
-I$(TRACEATLAS_ROOT)include/\
-I$(GSL_ROOT)include/
# this path specifies a static LLVM IR bitcode library to link to the program. All symbols used from this library will be profiled.
LIBRARIES=\
$(GSL_ROOT)lib/libgsl.a\
# This variable will be interpreted as a string and put into the command that runs the resulting program
RARGS=512

# Now that buildflow variables have been defined, we can include the buildflow driver Makefile
include BuildFlow.mk
```

## Rules
The "all" rule will run the entire existing TraceAtlas toolchain. This starts with compiling the program and ends with the LastWriter memory pass. Recent developments in the cartographer pass have broken the memory analysis passes, so in general don't expect them to work. For more information about each individual rule see the comments in BuildFlow.mk
