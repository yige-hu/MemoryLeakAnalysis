Memory Leak Analysis by Contradiction
====

Yige Hu (yh6596)

CS380c final project:

Memory leak analysis by contradiction.
It is an intra-procedural backward dataflow analysis to detect potential memory leakage on heap cells, implemented using LLVM.


References:
----

[Memory leak analysis by contradiction](http://dl.acm.org/citation.cfm?id=2090908)  
In Proceedings of the 13th international conference on Static Analysis (SAS'06), 2006.

The paper describes a backward dataflow analysis for both intra- and inter-procedual analysis. Due to the time constraint, only the intra-procedual analysis is implemented in this work.


Program catalog:
----

- Dataflow.h:

  A basic framework for Data-flow analysis.

- SetOperations.h:

  Some basic tools for set operations.

- LeakAnalysis.h:

  A memory-leak analysis.

- MemLeak.h & cpp:

  The leak pass.


Usage:
----

For the compatibility purpose with Jia's Andersen's analysis, LLVM is strongly suggested to be installed with v3.5, trunk 203470 .

- Install the Andersen's pointer analysis implemented by Jia:

  [https://github.com/grievejia/andersen](https://github.com/grievejia/andersen)

- Compile memLeak analysis:

  Change the "ANDERS\_DIR" in Makefile into the directory of the Andersen's analysis installed in step 1. Run:
```bash
  make memLeak
```

- Automatically run all the microbenchmarks:

  Change the "ANDERS\_DIR" in "run_microbench.sh" into the directory of the Andersen's analysis. Run:
```bash
  ./run_microbench.sh
```

- Analyze other C programs:

  Prepare for the .bc input:
```bash
  ./prepare.sh [c file names (without .c)]
```
  e.g.
```bash
  ./prepare.sh benchmark/glibc_math/atest-sincos
```
  Run memLeak pass:
```bash
  opt -load ../andersen/Debug+Asserts/lib/libAnders.so -load ./MemLeak.so -leak-pass atest-sincos.bc > /dev/null
```
