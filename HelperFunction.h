// CS380c final project: HelperFunction.h
// 
// Author: Yige Hu
////////////////////////////////////////////////////////////////////////////////

#ifndef HELPERFUNCTION_H
#define HELPERFUNCTION_H

#include "Dataflow.h"

using namespace llvm;

namespace {

bool implicitMiss(Instruction* inst, Triple trp) {

  return false;
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
