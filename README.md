# Algorithms
Just some algorithms that can be used as a reference to optimized algorithms for performance, profiling, code structuring, and other interesting things.

## TraceAtlas
The purpose of these algorithms is to test the capabilities of the TraceAtlas framework. Specifically these algorithms test how TraceAtlas can structure their code and recognize what they are doing. The build flow of this repository supports this framework. For more information about TraceAtlas, go to http://github.com/benroywillis/TraceAtlas

## Makefiles
Each Makefile in this repository facilitates the building of their respective programs using either a simple compilation pass (using GNU or LLVM C compilers) or the TraceAtlas analysis framework. For just building the program, make elf is the rule. For using the TraceAtlas framework, make all should do the trick.
