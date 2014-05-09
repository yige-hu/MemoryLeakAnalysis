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

  // General tools:

  ValSet getPt(const Value *val);

  ValSet getMem(const Value *val);

  bool disjoint(const Value *val, ValSet rs);

  // Helper functions:

  // 1. Assignment: *x0 <- x1
  bool implicitMiss(const Value *val, Triple trp);

  bool miss(const Value *val, Triple trp);

  bool infeasible(Triple trp);

  Triple cleanup(Triple trp);

  Triple getNewTrpByAssignment(Triple trp, Instruction *inst);

  Triple getNewTrpByAssignParams(Triple trp, Value *e0, Value *e1);

  // 2. Malloc(): *x0 <- malloc

  bool unaliasedHit(ValSet w, ValSet H);

  Triple getNewTrpByNullAssgn(Triple trp, Instruction *inst);

  // Dataflow virtual functions:

  virtual Triple getTop(int val_cnt, Instruction *probInst) {
    Triple init;

    if (isa<StoreInst>(probInst)) {

      Value *e0 = probInst->getOperand(1);
      Value *e1 = probInst->getOperand(0);
      init.S = getPt(e0);

      // Notice that here H stores AllocaInst, thus, symbolic addr instead of
      // pointer. Corrispondent changes also made in miss() and filter for H.
      init.H.push_back(e0);

      // Case #1: assignment, *e0 <- e1
      if (! isa<BitCastInst>(e1)) {
        init.M.push_back(e1);
      }

      // Case #2: allocations, *e0 <- malloc
      // push nothing into M - implicit miss

      /*
      if (isa<BitCastInst>(e1)) {
        BitCastInst *castInst = dyn_cast<BitCastInst>(e1);
        Value *callOp = castInst->getOperand(0);
        if (isa<CallInst>(callOp)) {
          CallInst *callInst = dyn_cast<CallInst>(callOp); 
          if (callInst->getCalledFunction()->getName() == "malloc") {

          }
        }
      }
      */

    } else if(isa<CallInst>(probInst)) {
      // Case #3: deallocations, free(e)
      CallInst *callInst = dyn_cast<CallInst>(probInst);
      if (callInst->getCalledFunction()->getName() == "free") {
        // TODO
      }
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
    (*final) = temp;

    if (isa<StoreInst>(inst)) {

      Value *e0 = inst->getOperand(1);
      Value *e1 = inst->getOperand(0);

#if 0
      // 2. Analysis malloc: *x0 <- malloc
      if (isa<BitCastInst>(e1)) {
        BitCastInst *castInst = dyn_cast<BitCastInst>(e1);
        Value *callOp = castInst->getOperand(0);
        if (isa<CallInst>(callOp)) {
          CallInst *callInst = dyn_cast<CallInst>(callOp); 
          if (callInst->getCalledFunction()->getName() == "malloc") {

            ValSet w = getPt(e0);
            ValSet H = temp.H;

            // malloc: (1) contradiction
            if (unaliasedHit(w, H) && disjoint(e0, w)) {
              for (ValSet::iterator it = H.begin(); it != H.end(); ++it) {
                if (isa<AllocaInst>(*it) && ((*it) == e0))
                  return true;
                if (isa<LoadInst>(*it) && (getSymAddr(*it) == e0))
                  return true;
              }
            }

            // malloc: (2) *e0 <- 0
            if (unaliasedHit(w, H) || (miss(e0, temp) && disjoint(e0, w))) {

              // additional_cnt = #stores_to_probInst
              if (e0 == cond_inst->getOperand(1)) {
                additional_cnt ++;
              }

              // for assignment: *x0 <- x1 
              newTriple = getNewTrpByNullAssgn(temp, inst);

              if (infeasible(newTriple)) {
                return true;
              } else {
                (*final) = cleanup(newTriple);
                return false;
              }

            }

            // malloc: (3) top
            {
              // should return a Top here
            }
          }
        }
      }
#endif

      // 1. Analysis assignments: *x0 <- x1

      // additional_cnt = #stores_to_probInst
      if (e0 == cond_inst->getOperand(1)) {
        additional_cnt ++;
      }

      // for assignment: *x0 <- x1 
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


  virtual bool additional_cond() {
    // if e0 in probInst is written/load only once, no mem-leak
    if (additional_cnt == 0) {
      errs() << "\tOnly load once:\n";
      return true;
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
  //ret |= isa<AllocaInst>(val);
  // store in H AllocaInst

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
  if (isa<AllocaInst>(val)) {
    // consider that H stores AllocaInst instead of pointer it self
    for (ValSet::iterator it = trp.M.begin(); it != trp.M.end(); ++it) {
      if (isa<LoadInst>(*it) && val == getSymAddr(*it)) {
        return true;
      }
    }
    //return false;
    return (! intersect(trp.S, getPt(val)));

  } else {
    return (belongsTo(val, trp.M) || implicitMiss(val, trp));
  }
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
  assert(isa<StoreInst>(inst) && "getNewTrpByAssng: inst is not a StoreInst!");

  Value *e0 = inst->getOperand(1);
  Value *e1 = inst->getOperand(0);

  return getNewTrpByAssignParams(trp, e0, e1);
}


Triple LeakAnalysis::getNewTrpByAssignParams(Triple trp, Value *e0, Value *e1) {
  Triple newTrp;
  ValSet w = getPt(e0);

  // S' = S U pt(e0)
  newTrp.S = trp.S;
  for (ValSet::iterator it = w.begin(); it != w.end(); ++it) {
    newTrp.S.push_back(*it);
  }

  // H' <- H
  // equivalent for the cases where H stores AllocaInst instead of the pointer
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
    if (disjoint(e1, w) && isa<LoadInst>(*it)
        && (belongsTo(e1, trp.H) || 
          // consider also e1's AllocaInst is stored insdead of itself
          (isa<LoadInst>(e1)&& belongsTo(getSymAddr(e1), trp.H)))
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
      if (belongsTo(e1, trp.H) ||
          // consider also e1's AllocaInst is stored insdead of itself
          (isa<LoadInst>(e1) && belongsTo(getSymAddr(e1), trp.H))) {
        newTrp.H.push_back(e1);
      }

      if (belongsTo(e1, trp.M)) {
        newTrp.M.push_back(e1);
      }
    }
  }

  // Subst1
  // TODO
  // should consider the case where H stores AllocaInst instead of the pointers

  // Subst2
  if (disjoint(e0, w) && (! intersect(getPt(e0), trp.S))) {
    newTrp.M.push_back(e1);
  }

  return newTrp;
}


bool LeakAnalysis::unaliasedHit(ValSet w, ValSet H) {
  // equivalent for the cases where H stores AllocaInst instead of the pointer
  for (ValSet::iterator it = H.begin(); it != H.end(); ++it) {

    // consider the case where H stores AllocaInst instead of the pointer
    if (isa<AllocaInst>(*it) && intersect(getPt(*it), w)) return true;

    // the case where the pointer itself is stored
    if (isa<LoadInst>(*it) && intersect(getPt(getSymAddr(*it)), w)) return true;
  } 

  return false;
}


Triple LeakAnalysis::getNewTrpByNullAssgn(Triple trp, Instruction *inst) {

  assert(isa<StoreInst>(inst) && "getNewTrpByAssng: inst is not a StoreInst!");

  Value *e0 = inst->getOperand(1);
  Value *e1 = NULL;
  //ConstantInt *C = ConstantInt::get(l->getType(), 0);

  return getNewTrpByAssignParams(trp, e0, e1);
}

}

#endif // LEAKANALYSIS_H

