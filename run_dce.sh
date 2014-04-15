#!/bin/bash -f

SOURCE=( test )
ITERATION=5

make

echo "----------------------------------------------"
echo "Peparations:"

for file in ${SOURCE[@]} 
do
  echo "Preparing $file.c ...."
  clang -O0 -emit-llvm -c $file.c
  opt -mem2reg $file.bc -o $file-simp.bc
  llvm-dis $file-simp.bc

  echo "DCE processing on $file-simp.bc ...."
  opt -load ./dce-pass.so -dce-pass $file-simp.bc -o $file-opt.bc
  llvm-dis $file-opt.bc

  echo "LLVM DCE processing on $file-simp.bc...."
  opt -dce $file-simp.bc -o $file-llvmopt.bc
  llvm-dis $file-llvmopt.bc

  for opt in 'simp' 'llvmopt' 'opt'
  do
    echo "Generating object $file-$opt...."
    clang $file-$opt.bc -O0 -o $file-$opt
  done

  echo
done

echo "Preparations finished."
echo "----------------------------------------------"
echo "Testing starts:"
echo "Each testcase run with $ITERATION iterations"
echo "    -simp: the unoptimized version"
echo "    -llvmopt: the LLVM standard DCE optimized version"
echo "    -opt: self-made DCE optimized version"
echo

for file in ${SOURCE[@]}
do
  echo ">>> Test case: $file"
  for opt in 'simp' 'llvmopt' 'opt'
  do
    echo "Running $file-$opt...."
    for i in $(eval echo {1..$ITERATION}) 
    do
      T="$(date +%s%N)"
      ./"$file-$opt"
      T="$(($(date +%s%N)-T))"
      M="$((M+T/1000000))"
    done
    M="$((M/ITERATION))"
    echo "Time (ms): ${M}"
  done

  echo
done
