#include "Passes.h"
#include "klee/Support/ErrorHandling.h"
#include "klee/Support/OptionCategories.h"
#include "llvm/IR/Mangler.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Casting.h"
#include <string>

using namespace llvm;

namespace {
static bool syscInsertions = false;
}

namespace klee {

char SystemCPass::ID;

bool SystemCPass::runOnModule(llvm::Module &M) {
  if(syscInsertions) return false;
  syscInsertions = true;
  Function *corpkg_create = M.getFunction("_ZN7sc_core18sc_cor_pkg_pthread6createEmPFvPvES1_");
  if(!corpkg_create) {
    klee_warning("[sysc-pass] could not find corpkg::create");
    return false;
  }
  Function *exec = M.getFunction("ptc_exec");
  if(!exec) {
    klee_warning("[sysc-pass] could not find pthread-create exec stub function");
    return false;
  }
  Function *pthread_create = M.getFunction("pthread_create");
  if(!pthread_create) {
    klee_warning("[sysc-pass] could not find pthread-create dummy");
    return false;
  }

  for(Function::iterator fi=corpkg_create->begin(),fe=corpkg_create->end();fi!=fe;++fi) {
    for(BasicBlock::iterator bi=fi->begin(),be=fi->end();bi!=be;++bi) {
      if(CallInst *call = dyn_cast<CallInst>(bi)) {
        Function *target = call->getCalledFunction();
        if(target && target == pthread_create) {
            Instruction *call_exec = call->clone();
            CallInst *call_exec_cast = dyn_cast<CallInst>(call_exec);
            call_exec_cast->setCalledFunction(exec);
            ++bi;
            bi->getParent()->getInstList().insert(bi, call_exec_cast);
            return true; // only one duplication possible, should be fine tho
        }
      }
    }
  }

  return false;
  // zukunft: mangled names vernünftig handeln
  // https://stackoverflow.com/questions/37874094/llvm-how-to-get-function-by-functions-real-original-name
}
} // namespace klee
