#ifndef _NL_BACKENDS_LLVM_CODEGEN_H_
#define _NL_BACKENDS_LLVM_CODEGEN_H_

#include <map>

#include "expr.h"
#include "visitor.h"

#include "llvm/IR/Value.h"

using llvm::Value;

class CodeGen : public ExprVisitor<void> {
public:
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
};

#endif // _NL_BACKENDS_LLVM_CODEGEN_H_
