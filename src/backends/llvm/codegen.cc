#include <map>
#include <memory>
#include <vector>

#include "codegen.h"

#include "expr.h"
#include "stmt.h"
#include "primitives.h"
#include "type_builder.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"

using llvm::BasicBlock;
using llvm::ConstantFP;
using llvm::ConstantInt;
using llvm::Function;
using llvm::FunctionType;
using llvm::Value;

void CodeGen::emit(const std::vector<Stmt *> &stmts) {
  for (const Stmt *stmt : stmts) {
    emit(stmt);
  }
}

void CodeGen::emit(const Stmt *stmt) { stmt->accept(this); }

Value *CodeGen::emit(const Expr *expr) {
  expr->accept(this);
  Value *val = expr_values[expr];
  expr_values.erase(expr);
  return val;
}

void CodeGen::visit(const Unary *expr) {
  auto r = emit(&expr->right);
  auto nl_type = expr_types[expr];
  if (nl_type == Primitives::Float()) {
    Value *m1 = ConstantFP::get(ctx, llvm::APFloat(-1.0));
    expr_values[expr] = builder->CreateFMul(m1, r, "negtmp");
  } else if (nl_type == Primitives::Int()) {
    llvm::Type *int_type = llvm::IntegerType::get(ctx, 32);
    Value *m1 = ConstantInt::get(int_type, -1);
    expr_values[expr] = builder->CreateMul(m1, r, "negtmp");
  }
}

void CodeGen::visit(const Binary *expr) {
  Value *l = emit(&expr->left);
  Value *r = emit(&expr->right);

  if (!l || !r)
    return;

  switch (expr->op.type) {
  case PLUS: {
    expr_values[expr] = builder->CreateFAdd(l, r, "addtmp");
    return;
  }
  case MINUS: {
    expr_values[expr] = builder->CreateFSub(l, r, "subtmp");
    return;
  }
  case STAR: {
    expr_values[expr] = builder->CreateFMul(l, r, "multmp");
    return;
  }
  case SLASH: {
    expr_values[expr] = builder->CreateFDiv(l, r, "divtmp");
    return;
  }
  case LESS: {
    // Here (and in the following comparisons), 'U' in 'ULT'
    // refers to 'unordered'. The actual instruction is
    // 'unordered or less than' - which corresponds to the 'uge'
    // condition code being the first arg to the fcmp instruction
    // in the generated IR. (llvm.org/docs/LangRef.html#id305)
    expr_values[expr] = builder->CreateFCmpULT(l, r, "cmp_lt_tmp");
    return;
  }
  case LESS_EQUAL: {
    expr_values[expr] = builder->CreateFCmpULE(l, r, "cmp_le_tmp");
    return;
  }
  case GREATER: {
    expr_values[expr] = builder->CreateFCmpUGT(l, r, "cmp_gt_tmp");
    return;
  }
  case GREATER_EQUAL: {
    expr_values[expr] = builder->CreateFCmpUGE(l, r, "cmp_ge_tmp");
    return;
  }
  case EQUAL_EQUAL: {
    // Use 'ordered and equal' here - 'ordered' means that
    // neither operand is QNAN (quiet NaN).
    expr_values[expr] = builder->CreateFCmpOEQ(l, r, "cmp_eq_tmp");
    return;
  }
  case BANG_EQUAL: {
    // Emit fcmp with 'unordered or not equal' condition code.
    expr_values[expr] = builder->CreateFCmpUNE(l, r, "cmp_ne_tmp");
    return;
  }
  default: {
    // Error
    expr_values[expr] = nullptr;
    return;
  }
  }
}

void CodeGen::visit(const NumLiteral *expr) {
  if (expr_types[expr] == Primitives::Float()) {
    expr_values[expr] = ConstantFP::get(ctx, llvm::APFloat(expr->as_double()));
  } else if (expr_types[expr] == Primitives::Int()) {
    llvm::IntegerType *int_type = llvm::IntegerType::get(ctx, 32);
    expr_values[expr] =
        ConstantInt::get(int_type, llvm::StringRef(expr->value), 10);
  }
}

void CodeGen::visit(const Grouping *expr) {
  expr_values[expr] = emit(&expr->expression);
}

void CodeGen::visit(const StrLiteral *expr) {
  expr_values[expr] = builder->CreateGlobalStringPtr(expr->value);
}

void CodeGen::visit(const BoolLiteral *expr) {
  expr_values[expr] =
      expr->value ? ConstantInt::getTrue(ctx) : ConstantInt::getFalse(ctx);
}

void CodeGen::visit(const Logical *expr) {
  // TODO: Implement short-circuit semantics here.
  Value *l = emit(&expr->left);
  Value *r = emit(&expr->right);

  if (!l || !r)
    return;

  switch (expr->op.type) {
  case AND: {
    expr_values[expr] = builder->CreateAnd(l, r, "and_tmp");
    return;
  }
  case OR: {
    expr_values[expr] = builder->CreateOr(l, r, "or_tmp");
    return;
  }
  default: { return; }
  }
}

void CodeGen::visit(const Variable *expr) {
  // expr_values[expr] = builder->CreateLoad(tb.to_llvm(expr_types[expr]), stores[expr]  );
}

void CodeGen::visit(const Assignment *expr) {}
void CodeGen::visit(const Call *expr) {}
void CodeGen::visit(const Get *expr) {}
void CodeGen::visit(const Set *expr) {}
void CodeGen::visit(const This *expr) {}

void CodeGen::visit(const ExprStmt *stmt) { emit(stmt->expression); }

void CodeGen::visit(const BlockStmt *stmt) {
  sm.enter();
  emit(stmt->block_contents);
  sm.exit();
}

void CodeGen::visit(const PrintStmt *stmt) {}
void CodeGen::visit(const VarStmt *stmt) {}

void CodeGen::visit(const ClassStmt *stmt) {
  std::string classname = stmt->name.lexeme;
  auto nl_type = sm.current().typetab->get(classname);
  llvm::Type *struct_type = tb.to_llvm(nl_type);

  // Hack: Generate a function so the IR for the class is dumped (for debug)
  // and not optimized away.
  std::vector<llvm::Type *> doubles(0, llvm::Type::getDoubleTy(ctx));
  FunctionType *ft = FunctionType::get(struct_type, doubles, false);
  Function::Create(ft, Function::ExternalLinkage, "class_func", module.get());
}

void CodeGen::visit(const IfStmt *stmt) {
  Value* cond = emit(stmt->condition); 
  if (!cond) return;
  cond = builder->CreateICmpNE(cond, ConstantInt::getFalse(ctx), "ifcond");

  Function* func = builder->GetInsertBlock()->getParent();
  BasicBlock* br_then = BasicBlock::Create(ctx, "then", func);
  BasicBlock* br_else = BasicBlock::Create(ctx, "else");
  BasicBlock* merge = BasicBlock::Create(ctx, "ifcont");

  builder->CreateCondBr(cond, br_then, br_else);

  builder->SetInsertPoint(br_then);
  emit(stmt->then_branch);
  builder->CreateBr(merge);

  func->getBasicBlockList().push_back(br_else);
  builder->SetInsertPoint(br_else);
  if (stmt->else_branch) emit(stmt->else_branch);
  // All BBs must be terminated (incl. fall-thru's) to pass verification.
  builder->CreateBr(merge);

  func->getBasicBlockList().push_back(merge);  
  builder->SetInsertPoint(merge);
}

void CodeGen::visit(const WhileStmt *stmt) {}

void CodeGen::visit(const FuncStmt *stmt) {
  auto key = TypeTableUtil::fn_key(stmt->name.lexeme);
  auto nl_functype = sm.current().typetab->get(key)->functype;

  llvm::Type *ret_type = tb.to_llvm(nl_functype->return_type);
  std::vector<llvm::Type *> arg_types;
  for (NLType nl_argtype : nl_functype->arg_types) {
    arg_types.push_back(tb.to_llvm(nl_argtype));
  }

  FunctionType *ft = FunctionType::get(ret_type, arg_types, false);
  Function *func = Function::Create(ft, Function::ExternalLinkage,
                                    stmt->name.lexeme, module.get());

  BasicBlock *BB = BasicBlock::Create(ctx, "entry", func);
  builder->SetInsertPoint(BB);

  sm.enter();
  emit(stmt->body);
  sm.exit();

  verifyFunction(*func);
}

void CodeGen::visit(const ReturnStmt *stmt) {
  if (stmt->value) {
    Value *val = emit(stmt->value);
    builder->CreateRet(val);
  } else {
    builder->CreateRetVoid();
  }
}
