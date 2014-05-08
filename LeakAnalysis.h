// CS380c final project: LeakAnalysis.h
// 
// Author: Yige Hu
////////////////////////////////////////////////////////////////////////////////

#ifndef LEAKANALYSIS_H
#define LEAKANALYSIS_H

//#include "Dataflow.h"
//#include "Andersen.h"
//#include "MemLeak.h"
#include "SetOperations.h"


using namespace llvm;

namespace {

/*
 * Memory-leak analysis.
 */
class LeakAnalysis : public DataFlowAnalysis<Triple, BACKWARDS> {
public:

  Andersen *anders;
  ValSet allocSites;

  LeakAnalysis() {}
  LeakAnalysis(Andersen *ads) {
    anders = ads;
    anders->getAllAllocationSites(allocSites);
  }


  // TODO: move in HelperFunctions.cpp

  ValSet getPt(const Value *val);

  ValSet getMem(const Value *val);

  bool disjoint(const Value *val, ValSet rs);

  bool implicitMiss(const Value *val, Triple trp);

  bool miss(const Value *val, Triple trp);

  bool infeasible(Triple trp);

  Triple cleanup(Triple trp);

  Triple getNewTrpByAssignment(Triple trp, Instruction *inst);



  virtual Triple getTop(int val_cnt, Instruction *probInst) {
    Triple init;

    if (isa<StoreInst>(probInst)) {
      // Assignment

      Value *e0 = probInst->getOperand(1);
      Value *e1 = probInst->getOperand(0);
      init.S = getPt(e0);
      init.H.push_back(e0);
      init.M.push_back(e1);
    } else if(isa<BitCastInst>(probInst)) {
      // Allocations
      // TODO
    } else if(isa<CallInst>(probInst)) {
      // Deallocations
      // TODO
    }

    return init;
  }

  virtual void meet(Triple *final, Triple *temp, BasicBlock *curr,
      BasicBlock *last) {

    final->S.insert(final->S.end(), temp->S.begin(), temp->S.end());
    final->H = getIntersect(final->H, temp->H);
    final->M = getIntersect(final->M, temp->M);
  }

  virtual bool transfer(Triple *final, Triple temp, Instruction *inst) {

    Triple newTriple;
    // TODO: PHINode ?

    if (isa<StoreInst>(inst)) {
      // 1. Analysis assignments

      newTriple = getNewTrpByAssignment(temp, inst);

      if (infeasible(newTriple)) {
        return true;
      } else {
        (*final) = cleanup(newTriple);
        return false;
      }

    } else if(isa<BitCastInst>(inst)) {
      // 2. Analysis of allocations
      // TODO
    } else if(isa<CallInst>(inst)) {
      // 3. Analysis of deallocations
      // TODO
    }

    return false;
  }

};


ValSet LeakAnalysis::getPt(const Value *val) {
  //errs() << "getPt: " << *val << '\n';
  assert(val->getType()->isPointerTy() && "getPt: val is not a pointer type!");
  ValSet ret;
  assert(anders->getPointsToSet(val, ret) &&
      "getPt: cannot get points to set of val.");
  return ret;
}


ValSet LeakAnalysis::getMem(const Value *val) {
  ValSet ret;

  // Mem(n) = Mem(a) = O
  if (isa<ConstantInt>(val) || isa<ConstantPointerNull>(val) ||
      isa<AllocaInst>(val)) {
    return ret;
  }

  // Mem(e.f) = Mem(e)
  // TODO

  // Mem(*e) = {*e} U Mem(e)
  if (isa<LoadInst>(val)) {
    ret = getMem(getSymAddr(val));
    ret.push_back(val);
  }

  // Mem(e0+e1) = Mem(e0) U Mem(e1)
  // TODO

}


bool LeakAnalysis::disjoint(const Value *val, ValSet rs) {
  ValSet mem = getMem(val);
  for (ValSet::iterator it = mem.begin(); it != mem.end(); ++it) {
    if (isa<LoadInst>(*it)) {
      if (intersect(getPt(getSymAddr(*it)), rs)) return false;
    }
  }
  return true;
}


bool LeakAnalysis::implicitMiss(const Value *val, Triple trp) {
  bool ret = false;
  // numerical constants
  ret |= isa<ConstantInt>(val);
  ret |= isa<ConstantPointerNull>(val);

  // symbolic addresses
  ret |= isa<AllocaInst>(val);

  // Invalid expressions
  // TODO

  // Lvalue expressions that represent regions outside of the region set S
  if (isa<LoadInst>(val)) {
    Value *val_addr = getSymAddr(val);
    assert(val_addr->getType()->isPointerTy() &&
        "getPt: val_addr is not a pointer type!");
    ret |= (! intersect(trp.S, getPt(val_addr)));
  }
  return ret;
}


bool LeakAnalysis::miss(const Value *val, Triple trp) {
  return (belongsTo(val, trp.M) || implicitMiss(val, trp));
}


bool LeakAnalysis::infeasible(Triple trp) {
  for (ValSet::iterator it = trp.H.begin(); it != trp.H.end(); ++it) {
    if (miss((*it), trp)) {
      return true;
    }
  } 
  return false;
}


Triple LeakAnalysis::cleanup(Triple trp) {
  Triple newTrp = trp;

  for (ValSet::iterator it = trp.M.begin(); it != trp.M.end(); ++it) {
    if (implicitMiss((*it), trp)) {
      newTrp.M.erase(std::remove(newTrp.M.begin(), newTrp.M.end(), *it),
          newTrp.M.end());
    }
  }

  return newTrp;
}


Triple LeakAnalysis::getNewTrpByAssignment(Triple trp, Instruction *inst) {
  Triple newTrp;

  assert(isa<StoreInst>(inst) && "getNewTrpByAssng: inst is not a StoreInst!");

  Value *e0 = inst->getOperand(1);
  Value *e1 = inst->getOperand(0);
  ValSet w = getPt(e0);

  // S' = S U pt(e0)
  newTrp.S = trp.S;
  for (ValSet::iterator it = w.begin(); it != w.end(); ++it) {
    newTrp.S.push_back(*it);
  }

  // H' <- H
  for (ValSet::iterator it = trp.H.begin(); it != trp.H.end(); ++it) {
    // Filter1
    if (disjoint(*it, w)) {
      newTrp.H.push_back(*it);
    }

    // Filter2
    if (disjoint(e1, w) && miss(e1, trp) && isa<LoadInst>(*it)
        && disjoint(getSymAddr(*it), w)) {
      newTrp.H.push_back(*it);
    }
  }

  // M' <- M
  for (ValSet::iterator it = trp.M.begin(); it != trp.M.end(); ++it) {
    // Filter1
    if (disjoint(*it, w)) {
      newTrp.M.push_back(*it);
    }

    // Filter3
    if (disjoint(e1, w) && belongsTo(e1, trp.H) && isa<LoadInst>(*it)
        && disjoint(getSymAddr(*it), w)) {
      newTrp.H.push_back(*it);
    }

    // Subst2
    if (disjoint(e0, w) && isa<LoadInst>(*it)
        && getSymAddr(*it) == e0) {
      newTrp.M.push_back(e1);
    }
  }

  // Filter4
  if (isa<LoadInst>(e1)) {
    Value *e1_addr = getSymAddr(e1);
    if (disjoint(e1_addr, w)) {
      if (belongsTo(e1, trp.H)) {
        newTrp.H.push_back(e1);
      }

      if (belongsTo(e1, trp.M)) {
        newTrp.M.push_back(e1);
      }
    }
  }

  // Subst1
  // TODO

  // Subst2
  if (disjoint(e0, w) && (! intersect(getPt(e0), trp.S))) {
    newTrp.M.push_back(e1);
  }

  return newTrp;
}


}

#endif // LEAKANALYSIS_H

