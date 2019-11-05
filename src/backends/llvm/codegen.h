#ifndef _NL_BACKENDS_LLVM_CODEGEN_H_
#define _NL_BACKENDS_LLVM_CODEGEN_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "cactus_table.h"
#include "expr.h"
#include "expr_types.h"
#include "visitor.h"
#include "scope_manager.h"
#include "type_builder.h"

#include "llvm/IR/Value.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

using llvm::Value;
using NamedValueTable = CactusTable<std::string, Value *>;

class CodeGen : public ExprVisitor<>, public StmtVisitor<> {
public:
  explicit CodeGen(ScopeManager &sm, ExprTypes expr_types)
      : sm(sm), expr_types(expr_types), tb(TypeBuilder(ctx)) {
    sm.reset(); // Go to initial (global) scope.
    module = llvm::make_unique<llvm::Module>("neeilang.main_module", ctx);
    builder = llvm::make_unique<llvm::IRBuilder<>>(ctx);
  }

  void emit(const std::vector<Stmt *> &stmts);
  void emit(const Stmt *stmt);
  Value *emit(const Expr *expr);
  void print() { module->print(llvm::errs(), nullptr); }

  EXPR_VISITOR_METHODS(void)
  STMT_VISITOR_METHODS(void)

private:
  ScopeManager &sm;
  ExprTypes expr_types;
  std::map<const Expr *, Value *> expr_values;
  llvm::LLVMContext ctx;
  std::unique_ptr<llvm::IRBuilder<>> builder = nullptr;
  TypeBuilder tb;
  std::unique_ptr<llvm::Module> module =
      nullptr; // Owns memory for generated IR.
  NamedValueTable named_vals;
  Value *codegen(Expr *expr);
  std::map<NLType, llvm::Type *> ll_types;
};

#endif // _NL_BACKENDS_LLVM_CODEGEN_H_
