// CS380c final project: MemLeak.h
// 
// Author: Yige Hu
////////////////////////////////////////////////////////////////////////////////

#ifndef MEMLEAK_H
#define MEMLEAK_H

#include "Dataflow.h"

using namespace llvm;

namespace {

typedef std::vector<const Value*> ValSet;

typedef struct triple_t {
  ValSet S;
  ValSet H;
  ValSet M;

  bool contradict = false;

  bool operator==(const triple_t& rhs) {
    bool ret = ((this->S == rhs.S) && (this->H == rhs.H) && (this->M == rhs.M)
        && (this->contradict == rhs.contradict));
    return ret;
  }

  bool operator!=(const triple_t& rhs) {
    return !((*this) == rhs);
  }

} Triple;


bool isLeakProb(Instruction *inst) {
  // assignment; malloc
  bool ret = (isa<StoreInst>(inst));
  //ret = (isa<StoreInst>(inst)) || (isa<BitCastInst>(inst))
  //          || ((isa<CallInst>(inst) && 0));
  return ret;
}

}

#endif // MEMLEAK_H
