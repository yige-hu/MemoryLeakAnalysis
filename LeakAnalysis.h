// CS380c final project: LeakAnalysis.h
// 
// Author: Yige Hu
////////////////////////////////////////////////////////////////////////////////

#ifndef LEAKANALYSIS_H
#define LEAKANALYSIS_H

#include "Dataflow.h"
#include "Andersen.h"
#include "MemLeak.h"


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


  // in HelperFunctions.cpp
  ValSet getPt(const Value *val);

  bool belongsTo(const Value *val, ValSet vs);

  bool intersect(ValSet vs1, ValSet vs2);

  ValSet getIntersect(ValSet vs1, ValSet vs2);

  ValSet getMem(const Value *val);

  bool disjoint(const Value *val, ValSet rs);

  bool implicitMiss(const Value *val, Triple trp);

  bool miss(const Value *val, Triple trp);

  bool infeasible(Triple trp);

  Triple cleanup(Triple trp);

  Triple getNewTripleByAssignment(Triple trp, Instruction *inst);


  virtual Triple getTop(int val_cnt, Instruction *probInst) {
    Triple init;

    if (isa<StoreInst>(probInst)) {
      Value *e0 = probInst->getOperand(1);
      Value *e1 = probInst->getOperand(0);
      init.S = getPt(e0);
      init.H.push_back(e0);
      init.M.push_back(e1);
    } else if(isa<BitCastInst>(probInst)) {
      // TODO
    } else if(isa<CallInst>(probInst)) {
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
      newTriple = getNewTripleByAssignment(temp, inst);

      if (infeasible(newTriple)) {
        return true;
      } else {
        (*final) = cleanup(newTriple);
        return false;
      }

    } else if(isa<BitCastInst>(inst)) {
      // TODO
    } else if(isa<CallInst>(inst)) {
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


bool LeakAnalysis::belongsTo(const Value *val, ValSet vs) {
  return (std::find(vs.begin(), vs.end(), val) != vs.end());
}


bool LeakAnalysis::intersect(ValSet vs1, ValSet vs2) {
  for (ValSet::iterator it = vs1.begin(); it != vs1.end(); ++it) {
    if (belongsTo((*it), vs2)) {
      return true;
    }
  }
  return false;
}


ValSet LeakAnalysis::getIntersect(ValSet vs1, ValSet vs2) {
  ValSet new_vs;
  for (ValSet::iterator it = vs1.begin(); it != vs1.end(); ++it) {
    if (belongsTo((*it), vs2)) {
      new_vs.push_back(*it);
    }
  }
  return new_vs;
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
    const Instruction *inst = dyn_cast<Instruction>(val);
    Value *val_addr = inst->getOperand(0);
    ret = getMem(val_addr);
    ret.push_back(val);
  }

  // Mem(e0+e1) = Mem(e0) U Mem(e1)
  // TODO

}

bool LeakAnalysis::disjoint(const Value *val, ValSet rs) {
  ValSet mem = getMem(val);
  for (ValSet::iterator it = mem.begin(); it != mem.end(); ++it) {
    if (isa<LoadInst>(*it)) {
      const Instruction *inst = dyn_cast<Instruction>(*it);
      Value *e_addr = inst->getOperand(0);
      if (intersect(getPt(e_addr), rs)) return false;
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
  //errs() << "getPt: " << *val << '\n';
  //assert(val->getType()->isPointerTy() && "getPt: val is not a pointer type!");
  if (isa<LoadInst>(val)) {
    const Instruction *inst = dyn_cast<Instruction>(val);
    Value *val_addr = inst->getOperand(0);
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


Triple LeakAnalysis::getNewTripleByAssignment(Triple trp, Instruction *inst) {
  Triple newTrp;

  Value *e0 = inst->getOperand(1);
  Value *e1 = inst->getOperand(0);

  // S' = S U pt(e0)
  ValSet e0_pt = getPt(e0);
  newTrp.S = trp.S;
  for (ValSet::iterator it = e0_pt.begin(); it != e0_pt.end(); ++it) {
    newTrp.S.push_back(*it);
  }

  // H'
  for (ValSet::iterator it = trp.H.begin(); it != trp.H.end(); ++it) {
    
  }

  // M'
  for (ValSet::iterator it = trp.M.begin(); it != trp.M.end(); ++it) {

  }

  return newTrp;
}


}

#endif // LEAKANALYSIS_H

