#include "MUSTSupportPass.h"
#include "Logger.h"
#include "MemOpVisitor.h"
#include "TypeUtil.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Module.h"

using namespace llvm;

#define DEBUG_TYPE "must"

namespace {
static llvm::RegisterPass<must::pass::MustSupportPass> msp("must", "MUST type information", false, false);
}  // namespace

// FIXME 1) include bitcasts? 2) disabled by default in LLVM builds (use LLVM_ENABLE_STATS when building)
// STATISTIC(NumInstrumentedMallocs, "Number of instrumented mallocs");
// STATISTIC(NumInstrumentedFrees, "Number of instrumented frees");
STATISTIC(NumFoundMallocs, "Number of detected mallocs");
STATISTIC(NumFoundFrees, "Number of detected frees");

namespace tu = util::type;

namespace must {
namespace pass {

// Used by LLVM pass manager to identify passes in memory
char MustSupportPass::ID = 0;

bool MustSupportPass::doInitialization(Module& m) {
  /**
   * Introduce the necessary instrumentation functions in the LLVM module.
   * functions:
   * void __must_support_alloc(void *ptr_base, int type_id, long int count, long int elem_size)
   * void __must_support_free(void *ptr)
   *
   * Also scan the LLVM module for type definitions and add them to our type list.
   */
  declareInstrumentationFunctions(m);

  propagateTypeInformation(m);

  return true;
}

bool MustSupportPass::doInitialization(Function& f) {
  // TODO Do we actually need a per-function initialization?
  return false;
}

bool MustSupportPass::runOnBasicBlock(BasicBlock& bb) {
  /*
   * + Find malloc functions
   * + Find free frunctions
   * + Generate calls to instrumentation functions
   */
  MemOpVisitor mOpsCollector;
  mOpsCollector.visit(bb);
  // instrument collected calls of bb:
  for (auto& malloc : mOpsCollector.listMalloc) {
    ++NumFoundMallocs;
  }
  for (auto& free : mOpsCollector.listFree) {
    ++NumFoundFrees;
  }
  return false;
}

bool MustSupportPass::doFinalization(Module& m) {
  /*
   * Persist the accumulated type definition information for this module.
   */
  LOG_INFO("MustSupportPass finished. Found (malloc/free): (" << NumFoundMallocs << "/" << NumFoundFrees << ")")
  return false;
}

void MustSupportPass::setFunctionLinkageExternal(llvm::Constant* c) {
  if (auto f = dyn_cast<Function>(c)) {
    assert(f != nullptr && "The function pointer is not null");
    f->setLinkage(GlobalValue::ExternalLinkage);
  }
}

void MustSupportPass::declareInstrumentationFunctions(Module& m) {
  auto& c = m.getContext();
  auto allocFunc = m.getOrInsertFunction(allocInstrumentation, tu::getVoidPtrType(c), tu::getInt32Type(c),
                                         tu::getInt64Type(c), tu::getInt64Type(c), nullptr);
  setFunctionLinkageExternal(allocFunc);

  auto freeFunc = m.getOrInsertFunction(freeInstrumentation, tu::getVoidPtrType(c), nullptr);
  setFunctionLinkageExternal(freeFunc);
}

void MustSupportPass::propagateTypeInformation(Module& m) {
  /* Read already acquired information from temporary storage */
  /*
   * Scan module for type definitions and add to the type information map
   * Type information needed:
   *  + Name
   *  + Data member
   *  + Extent
   *  + Our id
   */
}

}  // namespace pass
}  // namespace must
