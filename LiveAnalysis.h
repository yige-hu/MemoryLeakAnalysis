// Assignment 3: LiveAnalysis.h
// 
// Author: Yige Hu
////////////////////////////////////////////////////////////////////////////////

#ifndef LIVEANALYSIS_H
#define LIVEANALYSIS_H

#include "Dataflow.h"

using namespace llvm;

namespace {

/*
 * Liveness analysis.
 */
class LiveAnalysis : public DataFlowAnalysis<BitVector, BACKWARDS> {
public:

  virtual BitVector getTop(int val_cnt) {
    BitVector bv_init(val_cnt, false);
    return bv_init;
  }

  virtual void meet(BitVector *final, BitVector *temp, BasicBlock *curr,
      BasicBlock *last) {
    (*final) |= (*temp);

    for (BasicBlock::iterator i = last->begin(), ie = last->end(); 
        (i != ie) && (isa<PHINode>(i)); ++i) {
      for (User::op_iterator OI = i -> op_begin(), OE = i -> op_end();
          OI != OE; ++OI) {
        Value *val = *OI;
        PHINode *p = cast<PHINode>(i);
        if ((curr == p->getIncomingBlock(*OI))
            && (isa<Instruction>(val) || isa<Argument>(val))) {
          final->set(val_num[val]);
        }
      }
    }
  }

  virtual BitVector transfer(BitVector temp, Instruction *inst) {
    temp.reset(val_num[inst]);

    if (! isa<PHINode>(inst)) {
      for (User::op_iterator
          OI = inst -> op_begin(), OE = inst -> op_end(); OI != OE; ++OI) {
        Value *val = *OI;
        if (isa<Instruction>(val) || isa<Argument>(val)) {
          temp.set(val_num[val]);
        }
      }
    }

    return temp;
  }

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
};

class LiveAnnotator : public AssemblyAnnotationWriter {
public:
  LiveAnalysis &analysis;

  LiveAnnotator(LiveAnalysis &a) : analysis(a) {
  }

#if 1
  virtual void emitBasicBlockStartAnnot(const BasicBlock *bb,
      formatted_raw_ostream &os) {
    os << "; ";
    for(LiveAnalysis::const_iterator iter = analysis.in_begin(bb); 
        iter != analysis.in_end(bb); ++iter) {
      os.write_escaped((*iter)->getName());
      os << ", ";
    }
    os << "\n";
  }
#endif

#if 1
  virtual void emitInstructionAnnot(const Instruction *i,
      formatted_raw_ostream &os) {
    if (isa<PHINode>(i)) {
      return;
    }

    os << "; ";
    for(LiveAnalysis::const_iterator iter = analysis.in_begin(i); 
        iter != analysis.in_end(i); ++iter) {
      os.write_escaped((*iter)->getName());
      os << ", ";
    }
    os << "\n";
  }
#endif

};

}

#endif // LIVEANALYSIS_H
