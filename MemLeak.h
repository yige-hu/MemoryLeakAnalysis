// CS380c final project: MemLeak.h
// 
// Author: Yige Hu
////////////////////////////////////////////////////////////////////////////////

#ifndef MEMLEAK_H
#define MEMLEAK_H

#include "Dataflow.h"

using namespace llvm;

namespace {

typedef std::set<const Value*> ValSet;
typedef std::vector<const Value*> ValVector;

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
  bool ret = false;
  // Case #1: assignment; #2: allocation
  if (isa<StoreInst>(inst)) {
    if (isa<PointerType>(inst->getOperand(0)->getType())) {
      ret = true;
    }
  }

  // Case #3: deallocation -- dealing with field pointer lost
  // Fields of the struct cannot be accessed by name, but only by index.
  // This information is lacked when compiled with Clang.
  return ret;
}

}

#endif // MEMLEAK_H
