#include "backends/llvm/object.h"

#include "llvm/IR/Type.h"

int NL_OBJ_GC_BYTE_IDX = 0;
int NL_OBJ_VT_IDX = 1;

std::vector<llvm::Type *> object_header(llvm::LLVMContext &ctx) {
  return {llvm::Type::getInt8Ty(ctx), // GC byte
          // VTable ptr - this is essentially a void *, but
          // we'll bitcast this when codegen'ing method calls.
          llvm::PointerType::getUnqual(llvm::Type::getInt64PtrTy(ctx))};
}

int obj_header_size(llvm::LLVMContext &ctx) {
  return object_header(ctx).size();
}
