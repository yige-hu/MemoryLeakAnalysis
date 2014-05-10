// CS380c final project: LeakAnalysis.h
// 
// Author: Yige Hu
////////////////////////////////////////////////////////////////////////////////

#ifndef LEAKANALYSIS_H
#define LEAKANALYSIS_H

#include "llvm/IR/Type.h"
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

  Triple getNewTrpByAssgnParams(Triple trp, Value *e0, Value *e1);

  // 2. Allocation: *x0 <- malloc

  bool unaliasedHit(ValSet w, ValSet H);

  Triple getNewTrpByNullAssgn(Triple trp, StoreInst *inst);

  // 3. Deallocation: free(e)

  // 4. Condition: cond(e0 == e1)
  void getNewTrpByCond(Triple *trp, BranchInst *branchInst, CmpInst *cmpInst);

  void updateTrpByEqual(Triple *trp, Triple trp_succ, Value *e0, Value *e1);

  void updateTrpByNotEqual(Triple *trp, Triple trp_succ, Value *e0, Value *e1);



  // Dataflow virtual functions:

  virtual Triple getTop(int val_cnt, Instruction *probInst) {
    Triple init;
    init.contradict = false;

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

    } else if(isa<CallInst>(probInst)) {
      // Case #3: deallocations, free(e)
      // mainly deal with fields
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

    final->contradict &= temp->contradict;
  }

  virtual bool transfer(Triple *final, Triple temp, Instruction *inst) {

    Triple newTriple;
    (*final) = temp;

    if (StoreInst *storeInst = dyn_cast<StoreInst>(inst)) {

      Value *e0 = inst->getOperand(1);
      Value *e1 = inst->getOperand(0);

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
                if (isa<AllocaInst>(*it) && ((*it) == e0)) {
                  return true;
                }
                if (isa<LoadInst>(*it) && (getSymAddr(*it) == e0)) {
                  return true;
                }
              }
            }

            // malloc: (2) *e0 <- 0
            if (unaliasedHit(w, H) || (miss(e0, temp) && disjoint(e0, w))) {

              // for assignment: *x0 <- x1 
              newTriple = getNewTrpByNullAssgn(temp, storeInst);

              (*final) = cleanup(newTriple);
              if (infeasible(newTriple)) {
                return true;
              } else {
                return false;
              }

            }

            // malloc: (3) top
            // conservatively return a Top here
            // or can just move on and reach the entry
          }
        }
      }

      // 1. Analysis assignments: *x0 <- x1

      newTriple = getNewTrpByAssignment(temp, inst);

      (*final) = cleanup(newTriple);
      if (infeasible(newTriple)) {
        return true;
      } else {
        return false;
      }

    } else if(isa<CallInst>(inst)) {
      // 3. Analysis of deallocations: free(e)
      CallInst *callInst = dyn_cast<CallInst>(inst);
      if (callInst->getCalledFunction()->getName() == "free") {
        Value *bitCast = callInst->getArgOperand(0);
        assert(isa<BitCastInst>(bitCast) && "free(): type mismatch");
        Instruction *bitCastInst = dyn_cast<BitCastInst>(bitCast);

        Value *e = bitCastInst->getOperand(0);
        assert(isa<LoadInst>(e) && "free(): type mismatch");

        if ((belongsTo(e, temp.H)) || (belongsTo(getSymAddr(e), temp.H))) {
          // free: (1) contradiction
          return true;

        } else if (miss(e, temp)) {
          // free: (2) Miss(e, (S,H,M)), (S,H,M)
          // *final = temp

        } else {
          // free: (3), (S,H,M U {e})
          final->M.push_back(e);
        }
      }     
    } else if (BranchInst *branchInst = dyn_cast<BranchInst>(inst)) {
      // 4. Analysis of cond(e0 == e1)
      if (branchInst->isConditional()) {
        Value *cond = branchInst->getCondition();
        if (CmpInst *cmpInst = dyn_cast<CmpInst>(cond)) {
          if (cmpInst->isCommutative()) {
          //if (cmpInst->isTrueWhenEqual() || cmpInst->isFalseWhenEqual()) {

            if (cmpInst->isFalseWhenEqual())

            newTriple = temp;
            getNewTrpByCond(&newTriple, branchInst, cmpInst);

            (*final) = cleanup(newTriple);
            if (infeasible(newTriple)) {
              return true;
            } else {
              return false;
            }

          }
        }
      }
    }

    return false;
  }


  virtual bool additional_cond(Value *last_store) {
    // if e0 in probInst is written/load only once, no mem-leak
    return (last_store == cond_inst) || (additional_cnt <= 1);
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

  return getNewTrpByAssgnParams(trp, e0, e1);
}


Triple LeakAnalysis::getNewTrpByAssgnParams(Triple trp, Value *e0, Value *e1) {
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
  for (ValSet::iterator it = H.begin(); it != H.end(); ++it) {
    // consider the case where H stores AllocaInst instead of the pointer
    if (isa<AllocaInst>(*it) && (! intersect(getPt(*it), w)))
      return true;
    // the case where the pointer itself is stored
    if (isa<LoadInst>(*it) && (! intersect(getPt(getSymAddr(*it)), w)))
      return true;
  } 

  return false;
}


Triple LeakAnalysis::getNewTrpByNullAssgn(Triple trp, StoreInst *inst) {
  Value *e0 = inst->getOperand(1);
  ConstantInt *e1 = ConstantInt::get(Type::getInt64Ty(inst->getContext()), 0);

  return getNewTrpByAssgnParams(trp, e0, e1);
}


void LeakAnalysis::getNewTrpByCond(Triple *trp, BranchInst *branchInst,
    CmpInst *cmpInst) {

  Value *e0 = cmpInst->getOperand(0);
  Value *e1 = cmpInst->getOperand(1);

  BasicBlock *succ_then = branchInst->getSuccessor(0);
  BasicBlock *succ_else = branchInst->getSuccessor(1);

  Triple trp_then = blk_in_bv[succ_then];
  Triple trp_else = blk_in_bv[succ_else];

  if (succ_then->getSinglePredecessor()) {
    // cond(e0 == e1)
    if (cmpInst->isTrueWhenEqual()) {
      updateTrpByEqual(trp, trp_then, e0, e1);
    }

    // cond(e0 != e1)
    if (cmpInst->isFalseWhenEqual()) {
      updateTrpByNotEqual(trp, trp_then, e0, e1);
    }
  }

  if (succ_else->getSinglePredecessor()) {
    // cond(e0 == e1)
    if (cmpInst->isFalseWhenEqual()) {
      updateTrpByEqual(trp, trp_else, e0, e1);
    }

    // cond(e0 != e1)
    if (cmpInst->isTrueWhenEqual()) {
      updateTrpByNotEqual(trp, trp_else, e0, e1);
    }
  }
}


void LeakAnalysis::updateTrpByEqual(Triple *trp, Triple trp_succ,
    Value *e0, Value *e1) {

  // e0+ => +e1
  if (belongsTo(e0, trp_succ.H) ||
      (isa<LoadInst>(e0) && belongsTo(getSymAddr(e0), trp_succ.H))) {
      trp->H.push_back(e1);
  }

  // e1+ => +e0
  if (belongsTo(e1, trp_succ.H) ||
     (isa<LoadInst>(e1) && belongsTo(getSymAddr(e1), trp_succ.H))) {
      trp->H.push_back(e0);
  }

  // e0-- => -e1
  if (miss(e0, trp_succ)) {
    trp->M.push_back(e1);
  }

  // e1-- => -e0
  if (miss(e1, trp_succ)) {
    trp->M.push_back(e0);
  }
}


void LeakAnalysis::updateTrpByNotEqual(Triple *trp, Triple trp_succ,
    Value *e0, Value *e1) {

  // e0+ => -e1
  if (belongsTo(e0, trp_succ.H) ||
      (isa<LoadInst>(e0) && belongsTo(getSymAddr(e0), trp_succ.H))) {
    trp->M.push_back(e1);
  }

  // e1+ => -e0
  if (belongsTo(e1, trp_succ.H) ||
      (isa<LoadInst>(e1) && belongsTo(getSymAddr(e1), trp_succ.H))) {
    trp->M.push_back(e0);
  }
}

}

#endif // LEAKANALYSIS_H

