#ifndef _NL_BACKENDS_LLVM_TYPE_BUILDER_H_
#define _NL_BACKENDS_LLVM_TYPE_BUILDER_H_

#include <map>
#include <memory>

#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include "type.h"
#include "primitives.h"

class TypeBuilder {
public:
  explicit TypeBuilder(llvm::LLVMContext &ctx) : ctx(ctx) {
    ll_types[Primitives::Int()] = llvm::Type::getInt32Ty(ctx);
    ll_types[Primitives::Float()] = llvm::Type::getDoubleTy(ctx);
    ll_types[Primitives::Void()] = llvm::Type::getVoidTy(ctx);
  }

  llvm::Type *to_llvm(NLType t);

private:
  std::map<NLType, llvm::Type *> ll_types;
  llvm::LLVMContext &ctx;
};

#endif // _NL_BACKENDS_LLVM_TYPE_BUILDER_H_
