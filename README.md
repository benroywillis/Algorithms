# Algorithms
This repository provides a buildflow (using GNU Makefile) for the [Cyclebite](https://github.com/benroywillis/Cyclebite) toolchain. Its [publication](https://ieeexplore.ieee.org/document/10301361) provides insight to how it works and what its goals are.

## Quick start guide
 * Build [Cyclebite and its dependencies](https://github.com/benroywillis/Cyclebite/blob/devb/README.md) and install them somewhere.
 * Build [Halidev16](https://github.com/halide/Halide/releases/tag/v16.0.0) and install it somewhere (see our build instructions below).
 * Fill out the paths defined in Environment.mk. Variables after HALIDE_AUTOSCHEDULER are optional.
 * Go to the GEMM/Naive project and type "make". You should see the entire Cyclebite run to completion (Halide_generator.cpp and Halide_driver.cpp should be present)! If you don't see the halide files, something is wrong - please see FAQ or double-check that you installed your dependencies correctly.
 * Make a folder for your own project: FOO/Naive.
 * Copy GEMM/Naive/Makefile into your project folder.
 * Change the name of the "SOURCE" variable from "GEMM" to your project name: "FOO" (if your project is a C project, you can delete the "SUFFIX" variable inside the Makefile). See BuildFlow.mk for a description of each variable that appears in the Makefile.
 * Run "make" and Cyclebite will structure your program and export its task graph in Halide! If something goes wrong, see the FAQ section to see if your error is there. If it isn't, [submit an issue](https://github.com/benroywillis/Cyclebite/issues/new/).

## General Architecture
 * Buildflow.mk: provides the rules for building the toolchain, as well as state-of-the-art (SoA) program structuring, analysis, and optimization frameworks like [Halide](https://github.com/halide/Halide), [LLVM-Polly](polly.llvm.org), [Polygeist](polygeist.llvm.org), CUDA, and [OpenMP](openmp.llvm.org).
 * Environment.mk: holds the paths to dependencies used by the Cyclebite toolchain. Each variable is commented with a description of its intention - if it is marked required, you must have this dependency installed and pointed to for the Cyclebite toolchain to run to completion.
 * Each project is meant to build only itself. There is no global build framework (except the Compliance.py script - it collects information on how well Cyclebite structures and labels the tasks of the applications within this repo).
 * Within each project's "Makefile" file is a reference to Environment.mk and Buildflow.mk. Those imports provide the general configuration and buildflow rules to each project. Thus, to add your own project, make a directory for it, copy an existing project's Makefile (perhaps GEMM/Naive), change its include paths to match your project's location, and change "SOURCE" to fit the source files in your project. There are special rules for Halide projects (see "Halide").
 * You can build your project using the normal llvm front-end ("make run"), Cyclebite ("make"), LLVM-Polly (make run_polly), Polygeist (make run_cgeist), GNU gdb (make gdb), 

## Example: GEMM/Naive
This is a simple n-cubed square matrix multiply with some knobs to change the configuration of the application. There are no source dependencies for this file - it is a standalone application. If your application depends on a library, please see the GEMM/GSL example.

### Run Native Binary
To build the application toward the current platform, type
```command
make run
```

### Run The Cyclebite Toolchain
To build the current application using Cyclebite, run
```command
make
```
This will run the entire Cyclebite toolchain (including Cyclebite-Template, which ends in the export of Halide). During the running of the toolchain, many intermediate files will be generated.

#### Markov Profile (MP) and Epoch Profile (EP)  bitcodes and executables
 * .markov.bc: bitcode file annotated with the Markov Profile
 * .markov.native: executable that profiles the application's execution with the Markov Profile
 * .memory.bc: bitcode file annotated with the Epoch Profile
 * .memory.native: executable that localizes the application's epochs
 * .bin: state-transition table of .markov.native file exported after its execution
 * BlockInfo_<project>.json: holds information detailing which basic blocks called which in function indirection
 * Loop_<project>.json: holds information about the static loops of the application. This file is only used if hotcode is enabled in the cartographer stage
 * instance_<project>.json: contains the parent-most CGT candidates that have been upgraded to CGTs via the epoch localizer. Also contains communication patterns ("Communication" - key is producer and value(s) is consumer, the numbers are kernel IDs) and instruction footprint info ("Instruction2Footprint" - key is a valueID of the loadInst touching the footprint, value(s) is the unique ID of the footprint)
 * MemoryFootprint_<project>.json: contains the footprints touched by each task ID ("Hierarchy" is a task ID)
 * Memory_<project>.dot: contains the memory footprints of each task in the task graph
 * TaskGraph_<project>.dot: contains the task graph of the application. Both task and non-task epochs are included. Tasks are highlighted in blue dashed circles and are annotated with their task ID,frequency count. Communication patterns are dashed lines with their type annotated. Temporal relationships are solid lines pointing from predecessor to successor. Non-task epochs will have a unique identifier that doesn't lie on the task ID space.
 * TaskGraph_<project>.dot_taskonly: is the same as TaskGraph_<project>.dot except non-task epochs are ommitted.

If you run
```command
llvm-dis <project>.markov.bc
```
you will get a human-readable form of the Markov Profile (MP) annotated binary. Within this file are calls to the profiling backend: "MarkovIncrement( i64 )". The argument in this function call (i64) is the ID of the basic block it lies within. This will be helpful to understand the analysis of later Cyclebite toolchain stages.

#### Cartographer output files
The main output file from cartographer is kernel.json. This file contains a dictionary of many pieces of information, the most important being the "Kernels" dictionary. Inside "Kernels" are keys of IDs that belong to each individual kernel. Within a kernel ID is the "Blocks" list that contains all unique block IDs that belong to this kernel. Several other pieces of information, like performance intrinsics, the dynamic "Nodes" that represented the kernel in the segmentation algorithm, and others describe interesting characteristics about the kernel.

If hotcode detection is enabled, cartographer will output two additional kernel files: one with suffix .json_HC and another with suffix .json_HL. HC stands for hotcode, and its kernel file contains kernels constituting "hotblocks" from the profile. By default, the hotcode detection algorithm will sort blocks from greatest frequency to least, then gather all hot blocks until 95% of the total basic block execution frequency has been explained. To adjust this threshold, use the `-ht` option. HL stands for hotloop. A hotloop is a static loop that has at least one hot block in it. These two program segmentation schemes are intended to simulate state-of-the-art program segmentation techniques used in the computer architecture field.

If the repository is compiled with configuration `-DCMAKE_BUILD_TYPE=Debug` or `-DCMAKE_BUILD_TYPE=relwithdebinfo`, cartographer will output several additional files, many of which will have suffix `.dot`. These files encode the control flow graph of the program being analyzed at certain stages of the segmentation algorithm. These files can be converted in .svg graphics using [GraphViz](https://pypi.org/project/graphviz/) which can be [easily installed](https://graphviz.org/download/) on many linux distributions.

 * dot\_GEMM.dot: is a dot file of the final graph after all transformations have been applied (simplication transforms and CGT candidate localization).
 * DynamicCallGraph.dot: a dot file detailing the call graph of the application that was dynamically observed (thus, functions and function calls that did not occur during execution are omitted). This call graph will not contain any holes (that is, not nodes are floating - the call graph is one contiguous tree). Edges point from caller to callee.
 * FinalTransformedGraph.dot: is a dot file of the final graph after all transformations have been applied (simplication transforms and CGT candidate localization).
 * kernel_<project>.json: contains all coarse-grained task (CGT) candidates found in the simplified MCG by Cyclebite
 * kernel_<project>.json\_HotCode: contains all coarse-grained task (CGT) candidates found in the simplified MCG according to a HotCode (HC) structuring technique.
 * kernel_<project>.json\_HotLoop: contains all coarse-grained task (CGT) candidates found in the simplified MCG according to the HotLoop (HL) structuring technique.
 * LabeledMCG.dot: is a dot file of the raw state transition table with its basic block IDs annotated to each node. Each node is a basic block and each edge is either a branch or function call.
 * LastTransform.dot: is a dot file of the last graph simplication transform that took place (there is a comment at the top describing which simplication transform did the work). There are three graphs: (first) the subgraph that was transformed (before any transformations have been applied), (second) the entire MCG before transformation, and (third) the entire MCG after transformation.
 * LastVirtualizationTransform.dot: Details the last CGT candidation localization, or "virtualization", transform. There are three graphs just like LastTransform.dot.
 * MarkovControlGraph.dot: holds the unfiltered state transition table of MP. Nodes are basic blocks that were observed in the execution of the application and edges are either branch instructions or function calls (also observed in the execution of the application). The node IDs refer to the virtual node space in the Cartographer evaluation - they do not map to the basic block IDs implied in <project>.markov.ll
 * simplifiedMarkovControlGraph.dot: holds the fully-simplified MCG before its cycles are localized. Like MarkovControlGraph, the nodes in this graph lie in the cartographer's virtual node space, thus their IDs to not map to basic block IDs in <project>.markov.ll.
 * StaticCallGraph.dot: a dot file detailing the static configuration of the call graph in the application. In complex API applications, this graph will be huge and contain many nodes that don't connect to the tree. Edges point from caller to callee.
 * TransformedMarkovControlGraph_#.dot: contains the MCG of the application after # iterations of cycle localization. Like MCG, the node IDs in this graph do not map to the basic block IDs in <project>.markov.ll.

To map your tasks back to the source code lines which made them, run
```command
make SourceMap_KernelGrammar_<project>.json
```
This will produce two files:
 * SourceMap_project_instance.json: contains the tasks found by Cyclebite.
 * SourceMap_project_kernel.json: contains the task candidates that were found by Cyclebite.

Both files map each LLVM basic block ID to a source code line if possible.
If your tasks map to very few (or even no) source code lines, run your project again with max debug symbols:
```command
make clean
make OPFLAG=-O1 DEBUG=-g3
make SourceMap_<TabComplete>
```

#### Cyclebite-Template
Cyclebite-Template prints a variety of information when it's compiled with with `-DCMAKE_BUILD_TYPE="Debug"` or `-DCMAKE_BUILD_TYPE="relwithdebinfo"`. It will print the characteristics extracted from each task. Once each task is characterized, each task's parallel pattern label is printed. Finally, it outputs some files:
 * DFG_kernel#.dot: is a dot containing the annotated data flow of task #. The red nodes are state instructions, blue are memory instructions, and green are function instructions. Solid edges are dependencies and dashed edges are control edges with their dynamically-observed frequency and probabilities annotated. 
 * <project>.annotated_omp.<project_suffix>: is the user application with its parallel tasks annotated with OpenMP pragmas
 * Halide_driver.cpp: is a template for the exported Halide driver of the application. The user must change this file to feed the Halide pipeline correctly.
 * Halide_generator.cpp: is the automatically-exported Halide generator of the application's pipeline. The input and output tasks are ommitted from this file and must be placed in the Halide_driver.cpp file by the user.
 * IdxVarTree_Task#.dot: contains the multi-dimensional array accesses of memory within Task #. Edges point from child dimension to parent dimension (that is, if an array A[i][j], then j's outgoing edge points to node i).
 * Task\#\_Collection\#.dot: contains a rendering of the collection# in task #. Edges point from child dimension to parent dimension.

#### Generated Halide 
Cyclebite-Template generates two Halide files: 
 * Halide_generator.cpp: contains the exported application task graph as a [Halide generator](https://halide-lang.org/tutorials/tutorial_lesson_15_generators.html)
 * Halide_run.cpp: contains the driver for the exported Halide pipeline. The user must change the inputs inside this file to feed the pipeline in the same way they fed the pipeline in their original C/C++ program (and to output the results).

To build and run the generated Halide, follow these steps:
```command
mkdir Generated_Halide ; cd Generated_Halide
cp ../Halide_* .
mv Halide_generated.cpp your-project-name_generate.cpp
mv Halide_run.cpp your-project-name_run.cpp
cp /path-to-Algorithms-root/GEMM/Naive/KG_Halide_Generated/Makefile .
```
Next, you need to change the generated code inside your-project-name_run.cpp to feed the application pipeline and export its results like you did in your C/C++ program.
Finally, open the copied Makefile and change the project name, links, static configurations, and runtime arguments to match your project.
Then, 
```command
make run
```
and your halide will build and run!

To optimize your program fully and gather runtimes for it, pass the following variables when calling the Makefile:
 * OPFLAG=-O3 - set the optimization level of the front-end LLVM compiler that compiles the Halide program's generated LLVM IR. Set this to the highest level (O3) for optimal performance
 * DEBUG=-g0 - set this to no debug symbols (-g0) for optimal performance
 * HALIDE_THREADS=4 - defaults to 1. This is a dynamic flag to the Halide executable, so you can change this flag without rebuilding the Halide application
 * HALIDE_AUTOSCHEDULER=Anderson2021 - defaults to Adams2019 for CPU. If you want to compile toward your GPU, pass Anderson2021
 * TIMINGLIB_SAMPLES=15 - control how many timing samples will be collected for your program. Each sample is the arithmetic mean of all TIMINGLIB_ITERATIONS trials executed per sample
 * TIMINGLIB_ITERATIONS=15 - control how many iterations take place for each TIMINGLIB_SAMPLE
 * PRINT_TIMES=1 - print each time sample that is collected. Each measurement is in seconds.

An example of a configured Halide build-and-run for CPU:
```command
make clean ; make run TIMINGLIB_SAMPLES=15 TIMINGLIB_ITERATIONS=15 OPFLAG=-O3 DEBUG=-g0 PRINT_TIMES=1 HALIDE_THREADS=16
```
An example of a configured Halide build-and-run for GPU:
```command
make clean ; make run TIMINGLIB_SAMPLES=15 TIMINGLIB_ITERATIONS=15 OPFLAG=-O3 DEBUG=-g0 PRINT_TIMES=1 HALIDE_THREADS=16 HALIDE_AUTOSCHEDULER=Anderson2021
```

#### Halide autoschedulers
You have a choice to either export an application whose schedule is optimized for CPUs or GPUs. 
By default, the build flow will export a CPU-scheduled program.
You can change this by setting the HALIDE_AUTOSCHEDULER variable in Environment.mk
If you choose the GPU scheduler, you need to change the HALIDE_TARGET variable inside BuildFlow.mk to the cuda compatibility of your card (the default exports applications to an Nvidia RTX 3060 12GB).
Find your compatibility [here](https://developer.nvidia.com/cuda-gpus#compute).

## Halide
Cyclebite-Template exports the application task graph to the [Halide](https://people.csail.mit.edu/jrk/halide-pldi13.pdf) domain-specific language for transformation and optimization towards a [cpu](https://halide-lang.org/papers/autoscheduler2019.html) or [gpu](https://cseweb.ucsd.edu/~tzli/gpu_autoscheduler.pdf).

### Build Halide
We build Halide using the following configuration successfully with both gcc v11.4.0 and LLVM16 on Ubuntu 22.04LTS (it fails when using LLVM17: LLVM_Output.cpp:398:28: error: ‘createRewriteSymbolsPass’ is not a member of ‘llvm’; did you mean ‘RewriteSymbolPass’?). Our build flow:
```command
wget https://github.com/halide/Halide/archive/refs/tags/v16.0.0.tar.gz
mv v16.0.0.tar.gz Halide16.0.0.tar.gz
tar -xvf Halide16.0.0.tar.gz
cd Halide16.0.0
mkdir build_release ; cd build_release
cmake ../ -G Ninja -DCMAKE_BUILD_TYPE=Release -DLLVM_DIR=/path-to-llvm16-install/lib/cmake/llvm/ -DCMAKE_INSTALL_PREFIX=/path-to-Installs/Halide16/release/ -DWITH_TESTS=OFF
ninja
ninja install

```
Note: There is a compile problem with the Halide tests so those need to be turned off (FAILED: test/fuzz/CMakeFiles/fuzz_simplify.dir/simplify.cpp.o: error: current translation unit is compiled with the target feature '-fsanitize=fuzzer-no-link' but the AST file was not)

## FAQ
These are frequently asked questions, but a synonym would be "frequently encountered problems".
