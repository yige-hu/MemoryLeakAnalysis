// CS380c final project: HelperFunction.h
// 
// Author: Yige Hu
////////////////////////////////////////////////////////////////////////////////

#ifndef HELPERFUNCTION_H
#define HELPERFUNCTION_H

#include "Dataflow.h"

using namespace llvm;

namespace {

ValSet getPt(Andersen *anders, Instruction *inst) {
  assert(inst->getType()->isPointerTy() && "getPt: Inst is not a pointer type!");
  ValSet ret;
  assert(anders->getPointsToSet(inst, ret) &&
      "getPt: cannot get points to set of inst.");
  return ret;
}

bool intersect(ValSet vs1, ValSet vs2) {
  for (ValSet::iterator it = vs1.begin(); it != vs1.end(); ++it) {
    if (std::find(vs2.begin(), vs2.end(), (*it)) != vs2.end()) {
      return true;
    }
  }
  return false;
}

ValSet getIntersect(ValSet vs1, ValSet vs2) {
  ValSet new_vs;
  for (ValSet::iterator it = vs1.begin(); it != vs1.end(); ++it) {
    if (std::find(vs2.begin(), vs2.end(), (*it)) != vs2.end()) {
      new_vs.push_back(*it);
    }
  }
  return new_vs;
}

bool disjoint(Instruction *inst, ValSet rs) {
  // TODO
  return false;
}

bool implicitMiss(Andersen *anders, Instruction* inst, Triple trp) {
  bool ret = false;
  // numerical constant and symbolic addresses
  ret |= isa<ConstantInt>(inst);
  ret |= isa<ConstantPointerNull>(inst);
  // Invalid expressions
  // TODO
  // Lvalue expressions that represent regions outside of the region set S
  ret |= (! intersect(trp.S, getPt(anders, inst)));
  return ret;
}

bool miss(Instruction* inst, Triple trp) {

  return false;
}

bool infeasible(Triple trp) {

  return false;
}

Triple cleanup(Triple trp) {

}

Triple getNewTripleByAssignment(Triple trp, Instruction *inst) {
  Triple newTriple;

  Value *e0 = inst->getOperand(1);
  Value *e1 = inst->getOperand(0);

  return newTriple;
}

}

#endif // HELPERFUNCTION_H
