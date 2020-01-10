#include "backends/llvm/object.h"

#include "llvm/IR/Type.h"

std::vector<llvm::Type *> object_header(llvm::LLVMContext &ctx) {
  return {
      llvm::Type::getInt8Ty(ctx), // GC byte
      llvm::Type::getInt8PtrTy(
          ctx) // VTable ptr - this is essentially a void *, but
               // we'll bitcast this when codegen'ing method calls.
  };
}

int obj_header_size(llvm::LLVMContext &ctx) {
  return object_header(ctx).size();
}
