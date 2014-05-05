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

- LeakAnalysis.h:

  A memory-leak analysis.

- FunctionInfo.so:

  Print out the function info.

- MemLeak.cpp:

  The leak pass.


Usage:
----

1. Compile & prepare for the inputs:

  make
  ./prepare.sh [c file names]

2. Run:

  opt -load ./FunctionInfo.so -function-info pointer1-simp.bc

  opt -load ../andersen/Debug+Asserts/lib/libAnders.so -load ./SamplePass.so -sample-pass pointer1.ll > /dev/null

  opt -load ../andersen/Debug+Asserts/lib/libAnders.so -load ./MemLeak.so -leak-pass pointer1-simp.bc
