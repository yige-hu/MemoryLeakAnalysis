assignment3
====

Yige Hu
UTEID: yh6596
cs account: yige

Assignment 3 for CS380c



Dataflow.h:
  A basic framework for Data-flow analysis.
LiveAnalysis.h:
  A liveness analysis.
ReachAnalysis.h:
  A reaching defination analysis.
live.cpp:
  The live pass.
reach.cpp:
  The reach pass.



1. Compile & prepare for the inputs:
  make
  ./prepare.sh

2. Run:
Liveness:
  opt -load ./live.so -live sum-simp.bc > /dev/null

Reaching definations:
  opt -load ./reach.so -reach sum-simp.bc > /dev/null



* You could use the scripts 'prepare.sh' to prepare the inputs. Just change the source file names.
Otherwise, if you wanted to prepare input manually, run:

clang -O0 -emit-llvm -c loop.c
opt -mem2reg loop.bc -o loop-simp.bc

opt -load ../andersen/Debug+Asserts/lib/libAnders.so -load ./SamplePass.so -sample-pass sum.ll > /dev/null
opt -load ./FunctionInfo.so -function-info pointer1-simp.bc
