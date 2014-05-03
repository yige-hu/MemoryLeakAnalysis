#include "Andersen.h"

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/raw_ostream.h"

#include <vector>

using namespace llvm;

class SamplePass: public ModulePass
{
public:
	static char ID;

	SamplePass(): ModulePass(ID) {}
	bool runOnModule(Module &M);
	void getAnalysisUsage(AnalysisUsage &AU) const
	{
		AU.addRequired<Andersen>();
		AU.setPreservesAll();
	}
};

bool SamplePass::runOnModule(Module& M)
{
	Andersen& anders = getAnalysis<Andersen>();

	// Print all memory objects (represented by allocation sites)
	std::vector<const Value*> allocSites;
	anders.getAllAllocationSites(allocSites);
	errs() << "List of all memobj's allocation site:\n";
	for (auto const& val: allocSites)
		errs() << *val << "\n";

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
				// Note that the first argument passed to getPointsToSet() should be of a pointer type. Otherwise it will always return "I don't know"
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

	return false;
}

char SamplePass::ID = 0;
static RegisterPass<SamplePass> X("sample-pass", "Simple demonstration to show how to use Andersen's public interfaces", true, true);
