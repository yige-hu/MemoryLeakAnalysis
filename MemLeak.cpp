// Assignment 3: MemLeak.cpp
// 
// Author: Yige Hu
////////////////////////////////////////////////////////////////////////////////

#include "LeakAnalysis.h"

using namespace llvm;

namespace {

/*
 * The memory-leak analysis pass.
 */
class MemLeak : public FunctionPass {
public:
  static char ID;
  MemLeak() : FunctionPass(ID) {}

  virtual bool runOnFunction(Function &F) {
    LeakAnalysis analysis;

    // find all leak probing points and start backward dataflow
#if 0
    for (Function::iterator b = F.begin(), be = F.end(); b != be; ++b) {
      for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; ++i) {
        analysis.processFunction(F, i);
      }
    }
#endif

    analysis.processFunction(F);

    LeakAnnotator annot(analysis);
    F.print(errs(), &annot);
  }
};


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
