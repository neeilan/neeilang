#ifndef _NL_BACKENDS_LLVM_CODEGEN_H_
#define _NL_BACKENDS_LLVM_CODEGEN_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "cactus-table.h"
#include "expr-types.h"
#include "expr.h"
#include "scope-manager.h"
#include "type-builder.h"
#include "visitor.h"

#include "backends/abstract-codegen.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"

using llvm::Value;
using NamedValueTable = CactusTable<std::string, llvm::AllocaInst *>;

class CodeGen : public AbstractCodegen,
                public ExprVisitor<>,
                public StmtVisitor<> {
public:
  explicit CodeGen(ScopeManager &sm, ExprTypes expr_types)
      : sm(sm), expr_types(expr_types), tb(TypeBuilder(ctx)) {
    sm.reset(); // Go to initial (global) scope.
    module = llvm::make_unique<llvm::Module>("neeilang.main_module", ctx);
    builder = llvm::make_unique<llvm::IRBuilder<>>(ctx);
    init_libc();
  }

  void generate(const std::vector<Stmt *> &program);

  void print() { module->print(llvm::errs(), nullptr); }
  void write_bitcode();

  EXPR_VISITOR_METHODS(void)
  STMT_VISITOR_METHODS(void)

private:
  ScopeManager &sm;
  ExprTypes expr_types; // Typing information from type-checker
  std::map<const Expr *, Value *> expr_values;
  Value *last_deref_obj; // Last dereferenced object
  llvm::LLVMContext ctx;
  std::unique_ptr<llvm::IRBuilder<>> builder = nullptr;
  TypeBuilder tb;
  std::unique_ptr<llvm::Module> module =
      nullptr; // Owns memory for generated IR.
  std::shared_ptr<NamedValueTable> named_vals;
  NLType encl_class = nullptr;
  bool globals_only_pass = true; // Only codegen global classes and functions

  Value *codegen(Expr *expr);
  void enter_scope();
  void exit_scope();

  // libc bindings.
  void init_libc();
  void call_printf(llvm::Value *value, NLType t);

  void emit(const std::vector<Stmt *> &stmts);
  void emit(const Stmt *stmt);
  Value *emit(const Expr *expr);

  Value *get_int32(int value);

  // Virtual methods
  std::map<NLType, std::vector<llvm::Function *>> methods;
  llvm::Function *get_virtual_method(NLType type, const std::string &method);
  void build_vtables();
};

#endif // _NL_BACKENDS_LLVM_CODEGEN_H_
