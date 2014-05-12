#!/bin/bash -f

ANDERS_DIR=../andersen

files=`find microbenchmark/ -name "*.c"`
readarray -t sorted < <(printf '%s\0' "${files[@]}" | sort -z| xargs -0n1)

for f in "${sorted[@]}"
do
  f_name=`echo $f | sed "s/\.c$//"`
  f_name_base=`basename $f_name`
  ./prepare.sh $f_name
  cat $f
  echo "running memLeak analysis...."
  opt -load $ANDERS_DIR/Debug+Asserts/lib/libAnders.so -load ./MemLeak.so -leak-pass $f_name_base.bc > /dev/null
done
