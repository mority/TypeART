/*
 * MemOpVisitor.cpp
 *
 *  Created on: Jan 3, 2018
 *      Author: ahueck
 */

#include "MemOpVisitor.h"
#include "support/Logger.h"

#include <algorithm>

namespace must {
namespace pass {

using namespace llvm;

MemOpVisitor::MemOpVisitor() = default;

void MemOpVisitor::visitCallInst(llvm::CallInst& ci) {
  const auto isInSet = [&ci](const auto& funcSet) {
    const auto* f = ci.getCalledFunction();
    if (!f) {
      // TODO handle calls through, e.g., function pointers? - seems infeasible
      LOG_INFO("Encountered indirect call, skipping.");
      return false;
    }
    return std::find(std::begin(funcSet), std::end(funcSet), f->getName().str()) != std::end(funcSet);
  };

  if (isInSet(allocFunctions)) {
    visitMallocLike(ci);
  } else if (isInSet(deallocFunctions)) {
    visitFreeLike(ci);
  }
}

void MemOpVisitor::visitMallocLike(llvm::CallInst& ci) {
  LOG_DEBUG(ci.getCalledFunction()->getName());

  SmallVector<BitCastInst*, 4> bcasts;

  for (auto user : ci.users()) {
    // Simple case: Pointer is immediately casted
    if (auto inst = dyn_cast<BitCastInst>(user)) {
      bcasts.push_back(inst);
    }
    // Pointer is first stored, then loaded and subsequently casted
    if (auto storeInst = dyn_cast<StoreInst>(user)) {
      auto storeAddr = storeInst->getPointerOperand();
      for (auto storeUser : storeAddr->users()) {  // TODO: Ensure that load occurs ofter store?
        if (auto loadInst = dyn_cast<LoadInst>(storeUser)) {
          for (auto loadUser : loadInst->users()) {
            if (auto bcastInst = dyn_cast<BitCastInst>(loadUser)) {
              bcasts.push_back(bcastInst);
            }
          }
        }
      }
    }
  }

  LOG_DEBUG(">> number of bitcasts found: " << bcasts.size());

  listMalloc.push_back(MallocData{&ci, bcasts});
}

void MemOpVisitor::visitFreeLike(llvm::CallInst& ci) {
  LOG_DEBUG(ci.getCalledFunction()->getName());

  listFree.push_back(&ci);
}

void MemOpVisitor::visitAllocaInst(llvm::AllocaInst& ai) {
  LOG_DEBUG("Found alloca");

  // TODO filter based on indirect pointers (void *p = other_ptr)
  listAlloca.push_back(&ai);
}

MemOpVisitor::~MemOpVisitor() = default;

} /* namespace pass */
} /* namespace must */
