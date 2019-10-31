#include "expr.h"
#include "backends/llvm/codegen.h"

#include "llvm/IR/Value.h"

using llvm::Value;

Value* CodeGen::emit(const Expr *expr) {
  expr->accept(this);
  Value *val = expr_values[expr];
  expr_values.erase(expr);
  return val;
}

void CodeGen::visit(const Unary *expr) {}

void CodeGen::visit(const Binary *expr) {
  Value *l = emit(&expr->left);
  Value *r = emit(&expr->right);

  if (!l || !r) return;

  switch (expr->op.type) {
    case PLUS : {
      expr_values[expr] = builder->CreateFAdd(l, r, "addtmp");
      return;
    }
    case MINUS : {
      expr_values[expr] = builder->CreateFSub(l, r, "subtmp");
      return;
    }
    case STAR : {
      expr_values[expr] = builder->CreateFMul(l, r, "multmp");
      return;
    }
    case SLASH : {
      expr_values[expr] = builder->CreateFDiv(l, r, "divtmp");
      return;
    }
    default: {
      return;
    }
  }
}

void CodeGen::visit(const Grouping *expr) {}
void CodeGen::visit(const StrLiteral *expr) {}
void CodeGen::visit(const NumLiteral *expr) {}
void CodeGen::visit(const BoolLiteral *expr) {}
void CodeGen::visit(const Variable *expr) {}
void CodeGen::visit(const Assignment *expr) {}
void CodeGen::visit(const Logical *expr) {}
void CodeGen::visit(const Call *expr) {}
void CodeGen::visit(const Get *expr) {}
void CodeGen::visit(const Set *expr) {}
void CodeGen::visit(const This *expr) {}

