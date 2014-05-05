// Assignment 4: Dataflow.h
// 
// Author: Yige Hu
////////////////////////////////////////////////////////////////////////////////

#ifndef DATAFLOW_H
#define DATAFLOW_H

#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
//#include "llvm/Support/InstIterator.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"

#include "llvm/ADT/BitVector.h"
//#include "llvm/Assembly/AssemblyAnnotationWriter.h"
#include "llvm/IR/AssemblyAnnotationWriter.h"
#include "llvm/Support/FormattedStream.h"
//#include "llvm/DebugInfo.h"
#include "llvm/IR/DebugInfo.h"
//#include "llvm/Support/CFG.h"
//#include "llvm/IR/CFG.h"
#include "llvm/Analysis/CFG.h"

#include <ostream>
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>

using namespace llvm;

namespace {

enum Direction {
  FORWARDS,
  BACKWARDS,
};

/*
 * Data-flow analysis framework.
 */
template <typename _DOMAIN, Direction _DIR>
class DataFlowAnalysis {
public:
  typedef std::vector<Value*> DataFlowTypeList;

  typedef std::map<const BasicBlock*, DataFlowTypeList>::const_iterator blk_map_iter;
  typedef std::map<const Instruction*, DataFlowTypeList>::const_iterator inst_map_iter;

  typedef DataFlowTypeList::iterator iterator;
  typedef DataFlowTypeList::const_iterator const_iterator;

private:
  std::map<const Value*, DataFlowTypeList> InstValList;
  std::map<const BasicBlock*, DataFlowTypeList> BlkValList;

protected:
  int val_cnt;
  std::map<int, Value*> val_ptr;
  std::map<Value*, int> val_num;

  std::map<Value*, _DOMAIN> inst_in_bv;
  std::map<Value*, _DOMAIN> inst_out_bv;
  std::map<BasicBlock*, _DOMAIN> blk_out_bv;
  std::map<BasicBlock*, _DOMAIN> blk_in_bv;

  // for output purpose: contains possiblilities from all branches
  std::map<BasicBlock*, _DOMAIN> blk_in_bv_full;

public:

  // iterators
  iterator in_begin       (const BasicBlock *bb)
      { return BlkValList[bb].begin(); }

  iterator in_end         (const BasicBlock *bb)
      { return BlkValList[bb].end();   }
 
  iterator in_begin       (const Instruction *i) 
      { return InstValList[i].begin(); }

  iterator in_end         (const Instruction *i) 
      { return InstValList[i].end();   }
 
  const_iterator in_begin (const BasicBlock *bb) const
      {
        blk_map_iter iter = BlkValList.find(bb);
        return iter->second.begin();
      }

  const_iterator in_end   (const BasicBlock *bb) const
      {
        blk_map_iter iter = BlkValList.find(bb);
        return iter->second.end();
      }

  const_iterator in_begin (const Instruction *i) const
      {
        inst_map_iter iter = InstValList.find(i); 
        return iter->second.begin(); 
      }

  const_iterator in_end   (const Instruction *i) const
      { 
        inst_map_iter iter = InstValList.find(i);
        return iter->second.end();
      }

  virtual _DOMAIN getTop(int val_cnt, Instruction *probInst = NULL) {}

  virtual void boundary(_DOMAIN *entry, Function *F) {}

  virtual void meet(_DOMAIN *final, _DOMAIN *temp, BasicBlock *curr,
      BasicBlock *last) {}

  virtual bool transfer(_DOMAIN *final, _DOMAIN temp, Instruction *inst) {}

  /*
   * for output purpose only.
   * rewrite if need to include possibilities from all branches into blk_in_bv.
   */ 
  virtual void blk_in_full_proc(_DOMAIN *final, _DOMAIN *temp, BasicBlock *bb) {
    (*final) = (*temp);
  }

  virtual bool processFunction(Function &F, Instruction* probInst = NULL) {

    // initialization
    val_cnt = 0;

    {
      for (Function::arg_iterator a = F.arg_begin(), ae = F.arg_end();
          a != ae; a ++) {
        val_ptr[val_cnt] = a;
        val_num[a] = val_cnt;
        val_cnt ++;
      }

      for (Function::iterator b = F.begin(), be = F.end(); b != be; ++b) {
        for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; ++i) {
          val_ptr[val_cnt] = i;
          val_num[i] = val_cnt;
          val_cnt ++;
        }
      }

      _DOMAIN bv_init = getTop(val_cnt, probInst);

      int inst = 0;
      int blk = 0;

      for (Function::iterator b = F.begin(), be = F.end(); b != be; ++b) {
        blk_out_bv[b] = bv_init;
        blk_in_bv[b] = bv_init;
        blk_in_bv_full[b] = bv_init;
        for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; ++i) {
          inst_in_bv[i] = bv_init;
          inst_out_bv[i] = bv_init;
        }
      }
    }

    // processing using a worklist
    std::set<BasicBlock*> worklist;

    if (_DIR == BACKWARDS) {
      boundary(&blk_out_bv[--(F.end())], &F);

      if (probInst == NULL) {
        worklist.insert(--(F.end()));
      } else {
        worklist.insert(probInst->getParent());
      }

      while (!worklist.empty()) {

        BasicBlock* b = *worklist.begin();
        worklist.erase(b);
        _DOMAIN in_old = blk_in_bv[b];

        for (succ_iterator SI = succ_begin(b), SE = succ_end(b); SI != SE; ++SI) {
          BasicBlock *succ = * SI;
          meet(&blk_out_bv[b], &blk_in_bv[succ], b, succ);
        }

        _DOMAIN last;
        for (BasicBlock::reverse_iterator i = b->rbegin(), ie = b->rend();
            i != ie; ++i) {
          if (i == b->rbegin()) {
            last = blk_out_bv[b];
          }

          inst_out_bv[&*i] = last;
          if (transfer(&last, inst_out_bv[&*i], &*i)) return true;
          inst_in_bv[&*i] = last;
        }

        blk_in_bv[b] = last;
        blk_in_full_proc(&blk_in_bv_full[b], &blk_in_bv[b], b);

        // if in[] changed, add predecessors to worklist
        if (in_old != blk_in_bv[b]) {
          for (pred_iterator PI = pred_begin(b), PE = pred_end(b); PI != PE; ++PI) {
            BasicBlock *pred = * PI;
            worklist.insert(pred);
          }
        }
      } // while
    } else { // FORWARDS

      boundary(&blk_in_bv[F.begin()], &F);

      if (probInst == NULL) {
        worklist.insert(F.begin());
      } else {
        worklist.insert(probInst->getParent());
      }

      while (!worklist.empty()) {

        BasicBlock* b = *worklist.begin();
        worklist.erase(b);
        _DOMAIN out_old = blk_out_bv[b];

        for (pred_iterator PI = pred_begin(b), PE = pred_end(b); PI != PE; ++PI) {
          BasicBlock *pred = * PI;
          meet(&blk_in_bv[b], &blk_out_bv[pred], b, pred);
        }

        _DOMAIN last;
        for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; ++i) {
          if (i == b->begin()) {
            last = blk_in_bv[b];
          }

          inst_in_bv[i] = last;
          if (transfer(&last, inst_in_bv[i], i)) return true;
          inst_out_bv[i] = last;
        }

        blk_out_bv[b] = last;
        blk_in_full_proc(&blk_in_bv_full[b], &blk_in_bv[b], b);

        // if out[] changed, add successors to worklist
        if (out_old != blk_out_bv[b]) {
          for (succ_iterator SI = succ_begin(b), SE = succ_end(b); SI != SE; ++SI) {
            BasicBlock *succ = * SI;
            worklist.insert(succ);
          }
        }
      } // while
    } // if (_DIR)


#ifdef _FILL_ITERATOR
    // iterator filling: InstValList & BlkValList insertion
    for (typename std::map<Value*, _DOMAIN>::iterator
        it = inst_in_bv.begin(); it != inst_in_bv.end(); ++it) {
      for (int i = 0; i < val_cnt; i ++) {
        if (it->second[i]) {
          InstValList[it->first].push_back(val_ptr[i]);
        }
      }
    }

    for (typename std::map<BasicBlock*, _DOMAIN>::iterator
        it = blk_in_bv.begin(); it != blk_in_bv.end(); ++it) {
        //it = blk_in_bv_full.begin(); it != blk_in_bv_full.end(); ++it) {
      for (int i = 0; i < val_cnt; i ++) {
        if (it->second[i]) {
          BlkValList[it->first].push_back(val_ptr[i]);
        }
      }
    }
#endif

    // end of processFunction, return
    return false;
  }
};

} // end llvm namespace

#endif // DATAFLOW_H

