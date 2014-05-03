// Assignment 3: live.cpp
// 
// Author: Yige Hu
////////////////////////////////////////////////////////////////////////////////

#include "LiveAnalysis.h"

using namespace llvm;

namespace {

/*
 * The liveness pass.
 */
class Live : public FunctionPass {
public:
  static char ID;
  Live() : FunctionPass(ID) {}

  virtual bool runOnFunction(Function &F) {
    LiveAnalysis analysis;
    analysis.processFunction(F);

    LiveAnnotator annot(analysis);
    //F.print(errs(), &annot);
    F.print(errs());
  }
};


// LLVM uses the address of this static member to identify the pass, so the
// initialization value is unimportant.
char Live::ID = 0;

// Register this pass to be used by language front ends.
// This allows this pass to be called using the command:
//    clang -c -Xclang -load -Xclang ./Dataflow.so loop.c
static void registerMyPass(const PassManagerBuilder &,
                           PassManagerBase &PM) {
    PM.add(new Live());
}
RegisterStandardPasses
    RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                   registerMyPass);

// Register the pass name to allow it to be called with opt:
//    clang -c -emit-llvm loop.c
//    opt -load ./Dataflow.so -live loop.bc > /dev/null
// See http://llvm.org/releases/3.4/docs/WritingAnLLVMPass.html#running-a-pass-with-opt for more info.
RegisterPass<Live> X("live", "Data flow analysis: liveness");

}
