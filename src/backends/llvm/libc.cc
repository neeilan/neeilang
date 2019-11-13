#include <vector>

#include "backends/llvm/codegen.h"
#include "backends/llvm/libc.h"

static llvm::Constant *printfPtr;

void CodeGen::init_libc() {
  // printf
  std::vector<llvm::Type *> args;
  args.push_back(llvm::Type::getInt8PtrTy(ctx));

  // accepts a char*, is vararg, and returns int
  llvm::FunctionType *printfType =
      llvm::FunctionType::get(builder->getInt32Ty(), args, true);

  llvm::Constant *printfFunc =
      module->getOrInsertFunction("printf", printfType);

  printfPtr = printfFunc;
}

void CodeGen::call_printf(llvm::Value *value, NLType t) {
  llvm::Value *formatStr =
      builder->CreateGlobalStringPtr(NL_LLVM_libc::print_fmtstr(t));
  std::vector<llvm::Value *> values;
  values.push_back(formatStr);
  values.push_back(value);
  builder->CreateCall(printfPtr, values);
}
