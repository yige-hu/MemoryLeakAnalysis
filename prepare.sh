#!/bin/bash -f

for file in "$@"
do
  echo "preparing $file.c ...."
  clang -O0 -emit-llvm -c $file.c
  opt -mem2reg $file.bc -o $file-simp.bc
  llvm-dis $file.bc
  llvm-dis $file-simp.bc
done
