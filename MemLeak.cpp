// Assignment 3: MemLeak.cpp
// 
// Author: Yige Hu
////////////////////////////////////////////////////////////////////////////////

#include "llvm/IR/Module.h"

#include "LeakAnalysis.h"


using namespace llvm;

namespace {

/*
 * The memory-leak analysis pass.
 */
class MemLeak: public ModulePass {
public:
  static char ID;

  MemLeak(): ModulePass(ID) {}
  bool runOnModule(Module &M);
  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<Andersen>();
    AU.setPreservesAll();
  }
};

bool MemLeak::runOnModule(Module& M) {

  // run Anderson's analysis to get info

  Andersen& anders = getAnalysis<Andersen>();

  // Print all memory objects (represented by allocation sites)
  std::vector<const Value*> allocSites;
  anders.getAllAllocationSites(allocSites);

  errs() << "List of all memobj's allocation site:\n";
  for (auto const& val: allocSites) {
    errs() << *val << "\n";
  }
  errs() << "\n";

  // For each function, print the points-to set for each pointer it defines
  for (auto const& f: M)
  {
    errs() << "FUNCTION " << f.getName() << "\n";
    for (auto const& bb: f)
      for (auto const& inst: bb)
      {
        if (!inst.getType()->isPointerTy())
          continue;

        std::vector<const Value*> ptsSet;
        // Note that the first argument passed to getPointsToSet() should be
        // of a pointer type. Otherwise it will always return "I don't know"
        if (anders.getPointsToSet(&inst, ptsSet))
        {
          errs() << inst << "  -->>  ";
          for (auto const& val: ptsSet)
            errs() << *val << ", ";
          errs() << "\n";
        }
      }
    errs() << "\n";
  }

  // memory-leak detection

  LeakAnalysis analysis(&anders);

  // find all leak probing points, and start backward dataflow
  for (Module::iterator f = M.begin(), fe = M.end(); f != fe; ++f) {
    for (Function::iterator b = f->begin(), be = f->end(); b != be; ++b) {
      for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; ++i) {
        if (isLeakProb(i)) {
          bool retval = analysis.processFunction(*f, i);
          if (retval) {
            errs() << "Instruction: " << *i <<
                " is safe.\n";
          } else {
            errs() << "Instruction: '" << *i <<
                "' can cause a potential memory leakage.\n";
          }
        }
      }
    }
  }

  return false;
}

/*
class MemLeak : public FunctionPass {
public:
  static char ID;
  MemLeak() : FunctionPass(ID) {}

  virtual bool runOnFunction(Function &F) {

#if 1

#else
    //analysis.processFunction(F);
#endif

    LeakAnnotator annot(analysis);
    F.print(errs(), &annot);
  }
};
*/

// LLVM uses the address of this static member to identify the pass, so the
// initialization value is unimportant.
char MemLeak::ID = 0;

// Register this pass to be used by language front ends.
// This allows this pass to be called using the command:
//    clang -c -Xclang -load -Xclang ./Dataflow.so loop.c
static void registerMyPass(const PassManagerBuilder &,
                           PassManagerBase &PM) {
    PM.add(new MemLeak());
}
RegisterStandardPasses
    RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                   registerMyPass);

// Register the pass name to allow it to be called with opt:
//    clang -c -emit-llvm loop.c
//    opt -load ../andersen/Debug+Asserts/lib/libAnders.so -load
//        ./MemLeak.so -mem-leak pointer1.bc > /dev/null
RegisterPass<MemLeak> X("leak-pass", "memory leak analysis");

}
