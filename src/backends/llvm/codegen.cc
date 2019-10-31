#include "expr.h"
#include "backends/llvm/codegen.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/IR/Value.h"

using llvm::ConstantFP;
using llvm::ConstantInt;
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
    case LESS : {
      // Here (and in the following comparisons), 'U' in 'ULT'
      // refers to 'unordered'. The actual instruction is
      // 'unordered or less than' - which corresponds to the 'uge'
      // condition code being the first arg to the fcmp instruction
      // in the generated IR. (llvm.org/docs/LangRef.html#id305)
      expr_values[expr] = builder->CreateFCmpULT(l, r, "cmp_lt_tmp");
      return;
    }
    case LESS_EQUAL : {
      expr_values[expr] = builder->CreateFCmpULE(l, r, "cmp_le_tmp");
      return;
    }
    case GREATER : {
      expr_values[expr] = builder->CreateFCmpUGT(l, r, "cmp_gt_tmp");
      return;
    }
    case GREATER_EQUAL : {
      expr_values[expr] = builder->CreateFCmpUGE(l, r, "cmp_ge_tmp");
      return;
    }
    case EQUAL_EQUAL : {
      // Use 'ordered and equal' here - 'ordered' means that
      // neither operand is QNAN (quiet NaN).
      expr_values[expr] = builder->CreateFCmpOEQ(l,r, "cmp_eq_tmp");
      return;
    }
    case BANG_EQUAL : {
      // Emit fcmp with 'unordered or not equal' condition code.
      expr_values[expr] = builder->CreateFCmpUNE(l,r, "cmp_ne_tmp");
      return;
    }
    default: {
      // Error
      return;
    }
  }
}

void CodeGen::visit(const NumLiteral *expr) {
  expr_values[expr] = ConstantFP::get(ctx, llvm::APFloat(expr->value));
}

void CodeGen::visit(const Grouping *expr) {
  expr_values[expr] = emit(&expr->expression);
}

void CodeGen::visit(const StrLiteral *expr) {
  expr_values[expr] = builder->CreateGlobalStringPtr(expr->value);
}

void CodeGen::visit(const BoolLiteral *expr) {
  expr_values[expr] = expr->value ? ConstantInt::getTrue(ctx) : ConstantInt::getFalse(ctx);
}

void CodeGen::visit(const Logical *expr) {
  // TODO: Implement short-circuit semantics here.
  Value *l = emit(&expr->left);
  Value *r = emit(&expr->right);

  if (!l || !r) return;

  switch (expr->op.type) {
    case AND : {
      expr_values[expr] = builder->CreateAnd(l, r, "and_tmp");
      return;
    }
    case OR : {
      expr_values[expr] = builder->CreateOr(l, r, "or_tmp");
      return;
    }
    default : {
      return;
    }
  }
}

void CodeGen::visit(const Variable *expr) {}
void CodeGen::visit(const Assignment *expr) {}
void CodeGen::visit(const Call *expr) {}
void CodeGen::visit(const Get *expr) {}
void CodeGen::visit(const Set *expr) {}
void CodeGen::visit(const This *expr) {}

