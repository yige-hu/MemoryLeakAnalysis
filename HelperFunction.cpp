#include "LeakAnalysis.h"

using namespace llvm;

namespace {

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

}
