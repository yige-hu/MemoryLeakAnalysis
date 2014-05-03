// CS380c final project: MemLeak.h
// 
// Author: Yige Hu
////////////////////////////////////////////////////////////////////////////////

#ifndef MEMLEAK_H
#define MEMLEAK_H

#include "Dataflow.h"

using namespace llvm;

namespace {

typedef struct tuple_t {
  std::vector<Instruction*> S;
  std::vector<Instruction*> H;
  std::vector<Instruction*> M;
} Tuple;


}

#endif // MEMLEAK_H
