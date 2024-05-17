# Algorithms
This repository provides a buildflow (using GNU Makefile) for the [Cyclebite](https://github.com/benroywillis/Cyclebite) toolchain. Its [publication](https://ieeexplore.ieee.org/document/10301361) provides insight to how it works and what its goals are.

## Quick start guide
 * Build the [Cyclebite dependencies](https://github.com/benroywillis/Cyclebite/blob/main/README.md) and install them somewhere.
 * Fill out the paths defined in Environment.mk. Each variable is mandatory unless marked "optional:" in its comment.
 * Go to the GEMM/Naive project and type "make". You should see the entire Cyclebite run to completion (a KernelGrammar_\<project\>.json file should be present)! If you don't see the KernelGrammar file, something is wrong with your dependencies - please see FAQ or double-check that you installed your dependencies correctly.
 * Make a folder for your own project: FOO/Naive.
 * Copy GEMM/Naive/Makefile into your project folder.
 * Change the name of the "SOURCE" variable from "GEMM" to your project name: "FOO" (if your project is a C project, you can delete the "SUFFIX" variable).
 * Run "make" and Cyclebite will structure and export your project! If something goes wrong, see the FAQ section to see if your error is there. If it isn't, [submit an issue](https://github.com/benroywillis/Cyclebite/issues/new/).

## General Architecture
 * Buildflow.mk: provides the rules for building the toolchain, as well as state-of-the-art (SoA) program structuring, analysis, and optimization frameworks like [Halide](https://github.com/halide/Halide), [LLVM-Polly](polly.llvm.org), [Polygeist](polygeist.llvm.org), CUDA, and [OpenMP](openmp.llvm.org).
 * Environment.mk: holds the paths to dependencies used by the Cyclebite toolchain. Each variable is commented with a description of its intention - if it is marked required, you must have this dependency installed and pointed to for the Cyclebite toolchain to run to completion.
 * Each project is meant to build only itself. There is no global build framework (except the Compliance.py script - it collects information on how well Cyclebite structures and labels the tasks of the applications within this repo).
 * Within each project's "Makefile" file is a reference to Environment.mk and Buildflow.mk. Those imports provide the general configuration and buildflow rules to each project. Thus, to add your own project, make a directory for it, copy an existing project's Makefile (perhaps GEMM/Naive), change its include paths to match your project's location, and change "SOURCE" to fit the source files in your project. There are special rules for Halide project (see "Halide").

## Example: GEMM/Naive
This is a simple n-cubed square matrix multiply with some knobs to change the configuration of the application. There are no source dependencies for this file - it is a standalone application. If your application depends on a library, please see the GEMM/GSL example.

### Run Native Binary
To build the application toward the current platform, type
```console
:~$ make run
```

### Run The Cyclebite Toolchain
To build the current application using Cyclebite, run
```console
:~$ make
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
:~$ llvm-dis <project>.markov.bc
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

#### Cyclebite-Template
Cyclebite-Template prints a variety of information when it's compiled with with `-DCMAKE_BUILD_TYPE="Debug"` or `-DCMAKE_BUILD_TYPE="relwithdebinfo"`. It will print the characteristics extracted from each task. Once each task is characterized, each task's parallel pattern label is printed. Finally, it outputs some files:
 * DFG_kernel#.dot: is a dot containing the annotated data flow of task #. The red nodes are state instructions, blue are memory instructions, and green are function instructions. Solid edges are dependencies and dashed edges are control edges with their dynamically-observed frequency and probabilities annotated. 
 * <project>.annotated_omp.<project_suffix>: is the user application with its parallel tasks annotated with OpenMP pragmas
 * Halide_driver.cpp: is a template for the exported Halide driver of the application. The user must change this file to feed the Halide pipeline correctly.
 * Halide_generator.cpp: is the automatically-exported Halide generator of the application's pipeline. The input and output tasks are ommitted from this file and must be placed in the Halide_driver.cpp file by the user.
 * IdxVarTree_Task#.dot: contains the multi-dimensional array accesses of memory within Task #. Edges point from child dimension to parent dimension (that is, if an array A[i][j], then j's outgoing edge points to node i).
 * Task\#\_Collection\#.dot: contains a rendering of the collection# in task #. Edges point from child dimension to parent dimension.

## Halide
Halide projects are built with halide generators.

### Dependencies
 * [Halide v16](https://github.com/benroywillis/Halide/releases/tag/v16.0.0)

### Configure your project
 * Copy a simple halide folder (e.g., GEMM/Halide/) and name it after your project.

## FAQ
These are frequently asked questions, but a synonym would be "frequently encountered problems".
