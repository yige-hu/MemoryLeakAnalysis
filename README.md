Memory Leak Analysis by Contradiction
====

Yige Hu

CS380c final project:

Memory leak analysis by contradiction.
It is a backward dataflow analysis to detect potential memory leakage on heap cells, implemented using LLVM.

References:
----

[Memory leak analysis by contradiction](http://dl.acm.org/citation.cfm?id=2090908)  
In Proceedings of the 13th international conference on Static Analysis (SAS'06), 2006.


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

- * FunctionInfo.cpp:

  An auxiliary and independent tool to print out some function i

- * SamplePass.cpp:

  An auxiliary and independent tool to show the usage of the Andersen's analysis. Provided by Jia (grievejia).


Usage:
----

1. Install the Andersen's pointer analysis implemented by Jia:

  [grievejia/andersen](https://github.com/grievejia/andersen)

2. Compile & prepare for the inputs:

  make

  ./prepare.sh [c file names (without .c)]

  e.g. ./prepare.sh microbenchmark/1\_assignment/testcase1

3. Run:

  opt -load ./FunctionInfo.so -function-info testcase1.bc

  opt -load ../andersen/Debug+Asserts/lib/libAnders.so -load ./SamplePass.so -sample-pass testcase1.bc > /dev/null

  opt -load ../andersen/Debug+Asserts/lib/libAnders.so -load ./MemLeak.so -leak-pass testcase1.bc > /dev/null
