#ifndef _NL_BACKENDS_LLVM_CODEGEN_H_
#define _NL_BACKENDS_LLVM_CODEGEN_H_

#include <map>
#include <memory>
#include <string>

#include "cactus_table.h"
#include "expr.h"
#include "visitor.h"

#include "llvm/IR/Value.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

using llvm::Value;
using NamedValueTable = CactusTable<std::string, Value*>;

class CodeGen : public ExprVisitor<void> {
public:
  CodeGen() {
    builder = llvm::make_unique<llvm::IRBuilder<>>(ctx);
  }

  Value *emit(const Expr &expr) {
    expr.accept(this);
    return nullptr;
  }

  void visit(const Unary *);
  void visit(const Binary *);
  void visit(const Grouping *);
  void visit(const StrLiteral *);
  void visit(const NumLiteral *);
  void visit(const BoolLiteral *);
  void visit(const Variable *);
  void visit(const Assignment *);
  void visit(const Logical *);
  void visit(const Call *);
  void visit(const Get *);
  void visit(const Set *);
  void visit(const This *);

private:
  std::map<const Expr *, Value *> expr_values;
  llvm::LLVMContext ctx;
  std::unique_ptr<llvm::IRBuilder<>> builder = nullptr;
  std::unique_ptr<llvm::Module> module; // Owns memory for generated IR.
  NamedValueTable named_vals;
};

#endif // _NL_BACKENDS_LLVM_CODEGEN_H_
