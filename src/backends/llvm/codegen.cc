#include <cassert>
#include <map>
#include <memory>
#include <vector>

#include "codegen.h"

#include "expr.h"
#include "primitives.h"
#include "stmt.h"
#include "type-builder.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"

using llvm::AllocaInst;
using llvm::BasicBlock;
using llvm::CallInst;
using llvm::Constant;
using llvm::ConstantExpr;
using llvm::ConstantFP;
using llvm::ConstantInt;
using llvm::Function;
using llvm::FunctionType;
using llvm::Value;

#define NUMERIC_BINOP(type, instr_float, instr_int)                            \
  ((type == Primitives::Float())                                               \
       ? instr_float                                                           \
       : (type == Primitives::Int()) ? instr_int : nullptr);

static AllocaInst *entry_block_alloca(Function *fn, const std::string &s,
                                      llvm::Type *type) {
  llvm::IRBuilder<> builder(&fn->getEntryBlock(), fn->getEntryBlock().begin());
  return builder.CreateAlloca(type, 0, s);
}

Value *CodeGen::get_int32(int value) {
  llvm::Type *int_type = llvm::IntegerType::get(ctx, 32);
  return ConstantInt::get(int_type, value);
}

void CodeGen::generate(const std::vector<Stmt *> &program) {
  sm.reset();
  globals_only_pass = true;
  emit(program);

  sm.reset();
  globals_only_pass = false;
  emit(program);
}

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

  NLType l_ty = expr_types[&expr->left];
  NLType r_ty = expr_types[&expr->right];

  // Cast Int to Float for comparisons.
  if (l_ty == Primitives::Float() && r_ty == Primitives::Int()) {
    r = builder->CreateSIToFP(r, tb.to_llvm(Primitives::Float()));
    r_ty = Primitives::Float();
  } else if (r_ty == Primitives::Float() && l_ty == Primitives::Int()) {
    l = builder->CreateSIToFP(l, tb.to_llvm(Primitives::Float()));
    l_ty = Primitives::Float();
  }

  if (!l || !r)
    return;

  switch (expr->op.type) {
  // For the following instructions, we know both operands are Ints or Floats
  // (since we don't handle String concat with '+' yet).
  case PLUS: {
    expr_values[expr] =
        NUMERIC_BINOP(l_ty, builder->CreateFAdd(l, r, "addtmp"),
                      builder->CreateAdd(l, r, "addtmp")) return;
  }
  case MINUS: {
    expr_values[expr] =
        NUMERIC_BINOP(l_ty, builder->CreateFSub(l, r, "subtmp"),
                      builder->CreateSub(l, r, "subtmp")) return;
  }
  case GREATER: {
    expr_values[expr] =
        NUMERIC_BINOP(l_ty, builder->CreateFCmpUGT(l, r, "cmp_gt_tmp"),
                      builder->CreateICmpUGT(l, r, "cmp_gt_tmp")) return;
  }
  case GREATER_EQUAL: {
    expr_values[expr] =
        NUMERIC_BINOP(l_ty, builder->CreateFCmpUGE(l, r, "cmp_ge_tmp"),
                      builder->CreateICmpUGE(l, r, "cmp_ge_tmp")) return;
  }
  case LESS: {
    expr_values[expr] =
        NUMERIC_BINOP(l_ty, builder->CreateFCmpULT(l, r, "cmp_lt_tmp"),
                      builder->CreateICmpULT(l, r, "cmp_lt_tmp")) return;
  }
  case LESS_EQUAL: {
    expr_values[expr] =
        NUMERIC_BINOP(l_ty, builder->CreateFCmpULE(l, r, "cmp_lte_tmp"),
                      builder->CreateICmpULE(l, r, "cmp_lte_tmp")) return;
  }
  case EQUAL_EQUAL: {
    // Use 'ordered and equal' here - 'ordered' means that
    // neither operand is QNAN (quiet NaN).
    expr_values[expr] =
        NUMERIC_BINOP(l_ty, builder->CreateFCmpOEQ(l, r, "cmp_eq_tmp"),
                      builder->CreateICmpEQ(l, r, "cmp_eq_tmp")) return;
  }
  case BANG_EQUAL: {
    // Emit fcmp with 'unordered or not equal' condition code.
    expr_values[expr] =
        NUMERIC_BINOP(l_ty, builder->CreateFCmpUNE(l, r, "cmp_ne_tmp"),
                      builder->CreateICmpNE(l, r, "cmp_ne_tmp")) return;
  }
  case STAR: {
    expr_values[expr] =
        NUMERIC_BINOP(l_ty, builder->CreateFMul(l, r, "multmp"),
                      builder->CreateMul(l, r, "multmp")) return;
  }
  case SLASH: {
    expr_values[expr] =
        NUMERIC_BINOP(l_ty, builder->CreateFDiv(l, r, "divtmp"),
                      builder->CreateSDiv(l, r, "divtmp")) return;
  }
  default: {
    // Error
    assert(false && "Cannot generate IR for unknown binary operator");
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
  default: {
    return;
  }
  }
}

// Variables
static Value *emit_default_val(llvm::LLVMContext &ctx, NLType t) {
  if (t == Primitives::Bool())
    return ConstantInt::getFalse(ctx);
  if (t == Primitives::Float())
    return ConstantFP::get(ctx, llvm::APFloat(0.0));
  if (t == Primitives::Int())
    return ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0);
  return nullptr;
}

void CodeGen::visit(const VarStmt *stmt) {
  // TODO: Handle global variables.
  const std::string varname = stmt->name.lexeme;
  NLType nl_type = sm.current().typetab->get(varname);

  Function *fn = builder->GetInsertBlock()->getParent();
  Value *init = nullptr;
  if (stmt->expression) {
    init = emit(stmt->expression);
  } else {
    init = emit_default_val(ctx, nl_type);
    assert(init != nullptr && "Could not retrieve default value for type.");
  }

  AllocaInst *alloca = entry_block_alloca(fn, varname, tb.to_llvm(nl_type));
  builder->CreateStore(init, alloca);

  named_vals->insert(varname, alloca);
}

void CodeGen::visit(const Variable *expr) {
  const std::string varname = expr->name.lexeme;

  // HACK
  if (module->getFunction(varname)) {
    expr_values[expr] = module->getFunction(varname);
    return;
  }

  if (named_vals->contains(varname)) {
    expr_values[expr] =
        builder->CreateLoad(named_vals->get(varname), varname.c_str());
  }

  assert(expr_values[expr] != nullptr &&
         "No function or variable found with given name");
}

void CodeGen::visit(const Assignment *expr) {
  llvm::Value *value = emit(&expr->value);
  const std::string varname = expr->name.lexeme;
  builder->CreateStore(value, named_vals->get(varname));
  expr_values[expr] = value;
}

static bool is_initializer(const std::string &fn) {
  const std::string init("_init");
  if (fn.length() >= init.length()) {
    return (0 == fn.compare(fn.length() - init.length(), init.length(), init));
  } else {
    return false;
  }
}

void CodeGen::visit(const Call *expr) {
  Value *callee = emit(&expr->callee);
  assert(callee->getType()->isPointerTy() &&
         "Callee is not a (function) pointer");
  assert(static_cast<llvm::PointerType *>(callee->getType())
             ->getElementType()
             ->isFunctionTy() &&
         "Callee expr does not produce a function pointer");

  std::vector<Value *> args;

  // Initializer are special :)
  std::string fn_name = callee->getName();
  if (is_initializer(fn_name)) {
    const std::string classname = fn_name.substr(0, fn_name.find("_init"));
    auto nl_type = sm.current().typetab->get(classname);
    llvm::PointerType *ll_type =
        static_cast<llvm::PointerType *>(tb.to_llvm(nl_type));
    llvm::Type *ll_element_type = ll_type->getElementType();

    Constant *alloc_size = ConstantExpr::getSizeOf(ll_element_type);
    llvm::Instruction *malloc_instr = CallInst::CreateMalloc(
        builder->GetInsertBlock(), llvm::Type::getInt64Ty(ctx), ll_element_type,
        alloc_size, nullptr, nullptr, "malloced_" + classname);

    builder->Insert(malloc_instr);

    args.push_back(malloc_instr); // 'this' pointer.
  }

  for (const Expr *arg : expr->args) {
    args.push_back(emit(arg));
  }

  // Cannot attach a name ("calltmp") to void values, so no name here.
  expr_values[expr] = builder->CreateCall(callee, args);
}

void CodeGen::visit(const Get *expr) {
  auto field_name = expr->name.lexeme;

  NLType callee_nltype = expr_types[&expr->callee];
  assert(callee_nltype && "NL Type of callee unknown");

  // Initializers / static fields
  if (callee_nltype == Primitives::Class()) {
    const Variable *callee = static_cast<const Variable *>(&expr->callee);
    const std::string class_name = callee->name.lexeme;

    if (field_name == "init") {
      expr_values[expr] = module->getFunction(class_name + "_init");
    }
    return;
  }

  Value *callee = emit(&expr->callee);

  // Instance fields / methods
  const int field_idx = callee_nltype->field_idx(field_name);
  expr_values[expr] = builder->CreateLoad(
      builder->CreateGEP(callee, {get_int32(0), get_int32(field_idx)},
                         "fieldaccess_" + field_name),
      "load_" + field_name);
}

void CodeGen::visit(const Set *expr) {
  auto field_name = expr->name.lexeme;
  NLType callee_nltype = expr_types[&expr->callee];
  assert(callee_nltype && "NL Type of callee unknown");

  Value *callee = emit(&expr->callee);
  Value *value = emit(&expr->value);

  assert(value && "Set value cannot be null");

  const int field_idx = callee_nltype->field_idx(field_name);
  Value *elem_ptr =
      builder->CreateGEP(callee, {get_int32(0), get_int32(field_idx)},
                         "fieldaccess_" + field_name);

  // Instance fields / methods
  expr_values[expr] = builder->CreateStore(value, elem_ptr);
}

void CodeGen::visit(const This *expr) {
  expr_values[expr] = builder->CreateLoad(named_vals->get("this"), "this");
}

void CodeGen::visit(const ExprStmt *stmt) { emit(stmt->expression); }

void CodeGen::visit(const BlockStmt *stmt) {
  enter_scope();
  emit(stmt->block_contents);
  exit_scope();
}

void CodeGen::visit(const PrintStmt *stmt) {
  if (stmt->expression) {
    Value *value = emit(stmt->expression);
    call_printf(value, expr_types[stmt->expression]);
  }
}

void CodeGen::visit(const ClassStmt *stmt) {
  std::string classname = stmt->name.lexeme;
  auto nl_type = sm.current().typetab->get(classname);

  if (globals_only_pass) {
    tb.to_llvm(nl_type);
  }

  NLType prev_encl_class = encl_class;
  encl_class = nl_type;

  enter_scope();
  for (const Stmt *method : stmt->methods) {
    emit(method);
  }
  exit_scope();

  encl_class = prev_encl_class;
}

void CodeGen::visit(const IfStmt *stmt) {
  Value *cond = emit(stmt->condition);
  if (!cond)
    return;
  cond = builder->CreateICmpNE(cond, ConstantInt::getFalse(ctx), "ifcond");

  Function *func = builder->GetInsertBlock()->getParent();
  BasicBlock *br_then = BasicBlock::Create(ctx, "then", func);
  BasicBlock *br_else = BasicBlock::Create(ctx, "else");
  BasicBlock *merge = BasicBlock::Create(ctx, "ifcont");

  builder->CreateCondBr(cond, br_then, br_else);

  builder->SetInsertPoint(br_then);
  emit(stmt->then_branch);
  builder->CreateBr(merge);

  func->getBasicBlockList().push_back(br_else);
  builder->SetInsertPoint(br_else);
  if (stmt->else_branch)
    emit(stmt->else_branch);
  // All BBs must be terminated (incl. fall-thru's) to pass verification.
  builder->CreateBr(merge);

  func->getBasicBlockList().push_back(merge);
  builder->SetInsertPoint(merge);
}

void CodeGen::visit(const WhileStmt *stmt) {
  Function *func = builder->GetInsertBlock()->getParent();
  BasicBlock *check_cond = BasicBlock::Create(ctx, "check_cond", func);
  BasicBlock *loop = BasicBlock::Create(ctx, "loop", func);
  BasicBlock *post_loop = BasicBlock::Create(ctx, "post_loop", func);

  builder->CreateBr(check_cond);
  builder->SetInsertPoint(check_cond);
  Value *cond = emit(stmt->condition);
  builder->CreateCondBr(cond, loop, post_loop);

  builder->SetInsertPoint(loop);
  if (stmt->body) {
    emit(stmt->body);
  }

  builder->CreateBr(check_cond);
  builder->SetInsertPoint(post_loop);
}

void CodeGen::visit(const FuncStmt *stmt) {
  std::string fn_name = stmt->name.lexeme;
  if (encl_class) {
    fn_name = encl_class->name + "_" + fn_name;
  }
  if (globals_only_pass) {
    std::shared_ptr<FuncType> nl_functype;
    if (encl_class) {
      nl_functype = encl_class->get_method(stmt->name.lexeme);
    } else {
      auto key = TypeTableUtil::fn_key(stmt->name.lexeme);
      nl_functype = sm.current().typetab->get(key)->functype;
    }

    assert(nl_functype != nullptr &&
           "Cannot codegen function: FuncType not found.");

    llvm::Type *ret_type = tb.to_llvm(nl_functype->return_type);
    std::vector<llvm::Type *> arg_types;

    // Methods take object pointer as first arg
    if (encl_class) {
      arg_types.push_back(tb.to_llvm(encl_class));
    }

    for (NLType nl_argtype : nl_functype->arg_types) {
      arg_types.push_back(tb.to_llvm(nl_argtype));
    }

    FunctionType *ft = FunctionType::get(ret_type, arg_types, false);
    Function::Create(ft, Function::ExternalLinkage, fn_name, module.get());
    return;
  }

  if (!((encl_class && globals_only_pass) || !globals_only_pass))
    return;

  Function *func = module->getFunction(fn_name);
  auto arg_types = func->getFunctionType()->params();

  BasicBlock *entry = BasicBlock::Create(ctx, "entry", func);
  builder->SetInsertPoint(entry);

  std::vector<std::string> arg_names;
  if (encl_class)
    arg_names.push_back("this");
  for (auto &tok : stmt->parameters) {
    arg_names.push_back(tok.lexeme);
  }

  enter_scope();

  int arg_idx = 0;
  for (auto &arg : func->args()) {
    arg.setName(arg_names[arg_idx]);
    llvm::AllocaInst *alloca =
        entry_block_alloca(func, arg.getName(), arg_types[arg_idx]);
    arg_idx++;

    builder->CreateStore(&arg, alloca);
    named_vals->insert(arg.getName(), alloca);
  }

  emit(stmt->body);
  exit_scope();

  llvm::verifyFunction(*func);
}

void CodeGen::visit(const ReturnStmt *stmt) {
  if (stmt->value) {
    Value *val = emit(stmt->value);
    builder->CreateRet(val);
  } else {
    builder->CreateRetVoid();
  }
}

void CodeGen::visit(const GetIndex *expr) {}
void CodeGen::visit(const SetIndex *expr) {}
