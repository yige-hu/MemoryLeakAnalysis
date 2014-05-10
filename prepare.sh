#!/bin/bash -f

for file in "$@"
do
  echo "preparing $file.c ...."
  dir=`dirname $file`
  base=`basename $file`
  clang -O0 -emit-llvm -c $file.c
  # opt -mem2reg $file.bc -o $file-simp.bc
  llvm-dis $base.bc
  # llvm-dis $file-simp.bc
done
