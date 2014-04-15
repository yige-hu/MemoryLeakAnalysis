MemoryLeakAnalysis
==================

An implementation of the paper:
M. Orlovich and R. Rugina,
“Memory Leak Analysis by Contradi ction,” Static Analysis Sympo- sium, 2006.
http://citeseer.ist.psu.edu/viewdoc/summ ary?doi=10.1.1.120.5186


Yige Hu

Assignment 4 for CS380c

Please see the micro-benchmarks


(1) Programs

Dataflow.h:
  A basic framework for Data-flow analysis.
FVA.h:
  A Faint Variable Analysis.
dce-pass.cpp:
  The Dead Code Elimination pass.
licm-pass:
  The LICM pass.


(2) Scripts

run_dce.sh:
run_licm.sh:
  They will automatically compile, do optimizations on microbenchmarks and do run-time comparison, respectively for DCE and LICM.


(3) Microbenchmarks

test.c:
  For DCE.
loop1.c loop2.c loop3.c:
  For LICM.




* You could use the scripts 'run_dce.sh' and 'run_licm.sh' to prepare and run the microbenchmarks. Just change the source file names and the iteration number.
Otherwise, if you wanted to run manually:

1. Compile & prepare for the inputs:
  make
  clang -O0 -emit-llvm -c test.c
  opt -mem2reg $file.bc -o test-simp.bc

2. Run:
  opt -load ./dce-pass.so -dce-pass test-simp.bc -o test-opt.bc
  opt -load ./licm-pass.so -licm-pass loop1-simp.bc -o loop1-opt.bc
