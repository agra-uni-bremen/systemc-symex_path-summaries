#include "Passes.h"
#include "klee/Support/ErrorHandling.h"
#include "llvm/Support/Casting.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

namespace klee {

char LoopPass::ID;

void LoopPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<LoopInfoWrapperPass>();
}

bool LoopPass::runOnFunction(llvm::Function &F) {
  bool changed = false;
  LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  for(Loop *i : LI) {
    if(i->hasNoExitBlocks()) {
      // infinite: add dummy condition that always jumps into loop, but
      // fools the postdom computation into thinking an exit is possible
      BasicBlock *header = i->getHeader();

      BasicBlock *end = BasicBlock::Create(F.getContext(), "dummy.end", &F);
      ReturnInst::Create(F.getContext(),end);

      BasicBlock *cond = BasicBlock::Create(F.getContext(), "dummy.cond", &F, header);

      std::vector<Instruction*> change;
      for(BasicBlock &p : F) {
        if(!p.getTerminator()) continue;
        for(unsigned j=0;j<p.getTerminator()->getNumSuccessors();++j) {
          if(p.getTerminator()->getSuccessor(j) == header) {
            change.push_back(p.getTerminator());
          }
        }
//        if(&p==cond) continue;
        // should work since function checks if header is successor
//        p.getTerminator()->replaceSuccessorWith(header,cond);
      }
      for(Instruction *p : change) {
        p->replaceSuccessorWith(header,cond);
      }

      ConstantInt *t = ConstantInt::getTrue(F.getContext());
      BranchInst *branch = BranchInst::Create(header,end,t,cond);

      changed = true;
    }
  }
  return changed;
}
} // namespace klee
