// CS380c final project: SetOperations.h
// 
// Author: Yige Hu
////////////////////////////////////////////////////////////////////////////////

#ifndef SETOPERATIONS_H
#define SETOPERATIONS_H

#include "Dataflow.h"
#include "Andersen.h"
#include "MemLeak.h"


using namespace llvm;

namespace {


Value* getSymAddr(const Value *val) {
  assert(isa<LoadInst>(val) && "getSymAddr: val is not a LoadInst!");
  const Instruction *inst = dyn_cast<Instruction>(val);
  return inst->getOperand(0);
}


bool belongsTo(const Value *val, ValSet vs) {
  return (std::find(vs.begin(), vs.end(), val) != vs.end());
}


bool intersect(ValSet vs1, ValSet vs2) {
  for (ValSet::iterator it = vs1.begin(); it != vs1.end(); ++it) {
    if (belongsTo((*it), vs2)) {
      return true;
    }
  }
  return false;
}


ValSet getIntersect(ValSet vs1, ValSet vs2) {
  ValSet new_vs;
  for (ValSet::iterator it = vs1.begin(); it != vs1.end(); ++it) {
    if (belongsTo((*it), vs2)) {
      new_vs.insert(*it);
    }
  }
  return new_vs;
}

}

#endif // SETOPERATIONS_H
