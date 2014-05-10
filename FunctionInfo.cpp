// Assignment 2: FunctionInfo.cpp
// 
// Author: Yige Hu
////////////////////////////////////////////////////////////////////////////////

#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/Analysis/CFG.h"

#include <ostream>
#include <fstream>
#include <iostream>

using namespace llvm;

namespace {

class FunctionInfo : public ModulePass {
 private:
  std::map<Function*, int> callMap;

 public:
  static char ID;
  FunctionInfo() : ModulePass(ID) {}

  virtual bool runOnModule(Module &M) {

    for (Module::iterator f = M.begin(), fe = M.end(); f != fe; ++f) {
      callMap[f] = 0;
		}

    for (Module::iterator f = M.begin(), fe = M.end(); f != fe; ++f) {
      errs().write_escaped(f->getName()) << '\n';
      for (Function::iterator b = f->begin(), be = f->end(); b != be; ++b) {
        errs() << "\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>> block : "
            << b->getName() << '\n';
        for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; ++i) {
          errs() << *i << '\n';
          
#if 0
          if (isa<PHINode>(i)) {
            for (User::op_iterator OI = i -> op_begin(), OE = i -> op_end();
                OI != OE; ++OI) {
              Value *val = *OI;
              PHINode *p = cast<PHINode>(i);
              errs() << "\tbr: " << p->getIncomingBlock(*OI)->getName() << ": ";
              //errs() << "op: " << val->getName();
              errs() << "op: " << *val;
            }
          } else {
            for (User::op_iterator OI = i->op_begin(), OE = i-> op_end();
                OI != OE; ++OI) {
              Value *val = *OI;
              //errs() << "\top: " << val->getName();
              errs() << "\top: " << *val;
            }
          }
          errs() << '\n';
#endif
#if 0
          //if (isa<BranchInst>(i)) {
          if (isa<TerminatorInst>(i)) {
            errs() << "pred:------------\n";
            for (pred_iterator PI = pred_begin(b), PE = pred_end(b);
                PI != PE; ++PI) {
              //errs() << *(PI->begin()) << '\n';
              BasicBlock *B = * PI;
              errs() << '\t' << *(--(B->end()));
            }

            //succ_iterator SI(b), SE(b);
            errs() << "\nsucc:------------\n";
            for (succ_iterator SI = succ_begin(b), SE = succ_end(b);
                SI != SE; ++SI) {
              //errs() << *(PI->begin()) << '\n';
              BasicBlock *B = * SI;
              errs() << '\t' << *(B->begin());
            }
            errs() << "\n--------------\n";

          }
#endif

          if (CallInst* callInst = dyn_cast<CallInst>(&*i)) {
            if (callInst->getCalledFunction()->getName() == "malloc") {
              errs() << "\tis a malloc(), arg='" <<
                *(callInst->getArgOperand(0)) << "'\n";
            }
            if (callInst->getCalledFunction()->getName() == "free") {
              errs() << "\tis a free(), arg='" <<
                *(callInst->getArgOperand(0)) << "'\n";
            }

            if (callMap.count(callInst->getCalledFunction())) {
              callMap[callInst->getCalledFunction()] ++;
						} else {
							callMap[callInst->getCalledFunction()] = 1;
					  }
          }

          if (GetElementPtrInst* getElmPtrInst = dyn_cast<GetElementPtrInst>(&*i)) {
            errs() << "\tobject: " << *(getElmPtrInst->getPointerOperand()) << '\n';
          }

          if (BranchInst *branchInst = dyn_cast<BranchInst>(&*i)) {
            if (branchInst->isConditional()) {
              errs() << "\tCondition: " << *(branchInst->getCondition()) << '\n';
              //errs() << "\tsucc 0: " << *(branchInst->getSuccessor(0)) << '\n';
              //errs() << "\tsucc 1: " << *(branchInst->getSuccessor(1)) << '\n';
            }
          }

          if (StoreInst *storeInst = dyn_cast<StoreInst>(&*i)) {
            if (isa<PointerType>(storeInst->getOperand(0)->getType())) { 
              errs() << "\tstore to a pointer.\n"; 
            }
          }

        }
      }
    }

    for (Module::iterator F = M.begin(), Fe = M.end(); F != Fe; ++F) {
      errs().write_escaped(F->getName()) << ": arguments=";
		  errs() << F->getArgumentList().size() << ", call sites=";
  		errs() << callMap[F] << ", basic blocks=";
		  errs() << F->getBasicBlockList().size() << ", instructions=";

		  int inst_cnt = 0;
      for (inst_iterator I = inst_begin(*F), E = inst_end(*F); I != E; ++I) {
		    inst_cnt ++;
		  }
		  errs() << inst_cnt << '\n';
 
		}
  }
};

char FunctionInfo::ID = 0;

// Register this pass to be used by language front ends.
// This allows this pass to be called using the command:
//    clang -c -Xclang -load -Xclang ./FunctionInfo.so loop.c
static void registerMyPass(const PassManagerBuilder &,
                           PassManagerBase &PM) {
    PM.add(new FunctionInfo());
}
RegisterStandardPasses
    RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                   registerMyPass);

// Register the pass name to allow it to be called with opt:
//    clang -c -emit-llvm loop.c
//    opt -load ./FunctionInfo.so -function-info loop.bc > /dev/null
// See http://llvm.org/releases/3.4/docs/WritingAnLLVMPass.html#running-a-pass-with-opt for more info.
RegisterPass<FunctionInfo> X("function-info", "Function Information");

}
