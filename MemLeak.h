// CS380c final project: MemLeak.h
// 
// Author: Yige Hu
////////////////////////////////////////////////////////////////////////////////

#ifndef MEMLEAK_H
#define MEMLEAK_H

#include "Dataflow.h"

using namespace llvm;

namespace {

typedef struct triple_t {
  std::vector<Instruction*> S;
  std::vector<Instruction*> H;
  std::vector<Instruction*> M;
} Triple;


bool isLeakProb(Instruction *inst) {
  bool ret = (isa<LoadInst>(inst));
  //ret = (isa<LoadInst>(inst)) || (isa<BitCast>(inst)) || (isa<>(inst));
  return ret;
}

}

#endif // MEMLEAK_H
