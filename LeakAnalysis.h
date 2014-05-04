// CS380c final project: MemLeakAnalysis.h
// 
// Author: Yige Hu
////////////////////////////////////////////////////////////////////////////////

#ifndef LEAKANALYSIS_H
#define LEAKANALYSIS_H

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

  LeakAnalysis() {}
  LeakAnalysis(Andersen *ads) {
    anders = ads;
  }

  virtual Triple getTop(int val_cnt) {
    Triple init;
    // TODO
    return init;
  }

  virtual void meet(Triple *final, Triple *temp, BasicBlock *curr,
      BasicBlock *last) {
    //(*final) |= (*temp);
    // TODO

    for (BasicBlock::iterator i = last->begin(), ie = last->end(); 
        (i != ie) && (isa<PHINode>(i)); ++i) {
      for (User::op_iterator OI = i -> op_begin(), OE = i -> op_end();
          OI != OE; ++OI) {
        Value *val = *OI;
        PHINode *p = cast<PHINode>(i);
        if ((curr == p->getIncomingBlock(*OI))
            && (isa<Instruction>(val) || isa<Argument>(val))) {
          //final->set(val_num[val]);
          // TODO
        }
      }
    }
  }

  virtual Triple transfer(Triple temp, Instruction *inst) {
    //temp.reset(val_num[inst]);
    // TODO

    if (! isa<PHINode>(inst)) {
      for (User::op_iterator
          OI = inst -> op_begin(), OE = inst -> op_end(); OI != OE; ++OI) {
        Value *val = *OI;
        if (isa<Instruction>(val) || isa<Argument>(val)) {
          //temp.set(val_num[val]);
          // TODO
        }
      }
    }

    return temp;
  }

#if 0
  virtual void blk_in_full_proc(BitVector *final, BitVector *temp, BasicBlock *bb) {
    (*final) = (*temp);

    for (BasicBlock::iterator i = bb->begin(), ie = bb->end(); 
        (i != ie) && (isa<PHINode>(i)); ++i) {
      for (User::op_iterator OI = i -> op_begin(), OE = i -> op_end();
          OI != OE; ++OI) {
        Value *val = *OI;
        if (isa<Instruction>(val) || isa<Argument>(val)) {
          final->set(val_num[val]);
        }
      }
    }
  }
#endif

};

class LeakAnnotator : public AssemblyAnnotationWriter {
public:
  LeakAnalysis &analysis;

  LeakAnnotator(LeakAnalysis &a) : analysis(a) {
  }

#if 1
  virtual void emitBasicBlockStartAnnot(const BasicBlock *bb,
      formatted_raw_ostream &os) {
      os << "; ";
      for(LeakAnalysis::const_iterator iter = analysis.in_begin(bb); 
          iter != analysis.in_end(bb); ++iter) {
        os.write_escaped((*iter)->getName());
        os << ", ";
      }
      os << "\n";
   }
#endif

#if 1
  virtual void emitInstructionAnnot(const Instruction *i, formatted_raw_ostream &os) {
    if (isa<PHINode>(i)) {
      return;
    }

    os << "; ";
    for(LeakAnalysis::const_iterator iter = analysis.in_begin(i); 
        iter != analysis.in_end(i); ++iter) {
      os.write_escaped((*iter)->getName());
      os << ", ";
    }
    os << "\n";
  }
#endif

};

}

#endif // LEAKANALYSIS_H

