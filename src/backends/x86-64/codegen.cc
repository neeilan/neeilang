#include "backends/x86-64/codegen.h"
#include "backends/x86-64/asm.h"

#include <cassert>

using CodeGen = X86_64::CodeGen;

void CodeGen::asm_emit(std::vector<const char *> args) {
  assert(args.size() > 0 && args.size() <= 3 &&
         "Asm instr + args must number 1 - 3.");
  if (args.size() == 1) {
    asm_create_instr_only(&asm_tail, args[0]);

  } else if (args.size() == 2) {
    asm_create_1arg(&asm_tail, args[0], args[1]);
  } else {
    asm_create(&asm_tail, args[0], args[1], args[2]);
  }
}

void CodeGen::generate(const std::vector<Stmt *> &program) {
  emit(program);
  asm_print(stderr, asm_start(asm_tail));
}

void CodeGen::emit(const std::vector<Stmt *> &stmts) {
  for (const Stmt *stmt : stmts)
    emit(stmt);
}

void CodeGen::emit(const Stmt *stmt) { stmt->accept(this); }

void CodeGen::emit(const Expr *expr) { expr->accept(this); }

void CodeGen::visit(const ExprStmt *stmt) { emit(stmt->expression); }

void CodeGen::visit(const BlockStmt *) {}
void CodeGen::visit(const PrintStmt *) {}
void CodeGen::visit(const VarStmt *) {}
void CodeGen::visit(const ClassStmt *) {}
void CodeGen::visit(const IfStmt *) {}
void CodeGen::visit(const WhileStmt *) {}
void CodeGen::visit(const FuncStmt *) {}
void CodeGen::visit(const ReturnStmt *) {}

void CodeGen::visit(const Unary *) {}

void CodeGen::visit(const Binary *expr) {
  emit(&expr->left);
  emit(&expr->right);

  asm_emit({"pop", "rdi"}); // right
  asm_emit({"pop", "rax"}); // left

  switch (expr->op.type) {
  case PLUS: {
    asm_emit({"add", "rax", "rdi"});
    break;
  }
  case MINUS: {
    asm_emit({"sub", "rax", "rdi"});
    break;
  }
  case STAR: {
    asm_emit({"imul", "rax", "rdi"});
    break;
  }
  case SLASH: {
    /*
    idiv takes RDX and RAX, and divides the sum by
    the arg register value.
    Quotient is stored into RAX, remainder in RDX.
    The CQO instruction (available in 64-bit mode only) copies the
    sign (bit 63) of the value in the RAX register into every bit
    position in the RDX register (Intel 64 and IA-32 Architectures
    Software Developer’s Manual
    */
    asm_emit({"cqo"});
    asm_emit({"idiv", "rdi"});
    break;
  }
  default:
    return;
  }
  asm_emit({"push", "rax"});
}

void CodeGen::visit(const Grouping *expr) { emit(&expr->expression); }

void CodeGen::visit(const StrLiteral *) {}

void CodeGen::visit(const NumLiteral *expr) {
  asm_emit({"push", expr->value.c_str()});
}

void CodeGen::visit(const BoolLiteral *expr) {
  if (expr->value)
    asm_emit({"push", "1"});
  else
    asm_emit({"push", "0"});
}

void CodeGen::visit(const Variable *) {}
void CodeGen::visit(const Assignment *) {}
void CodeGen::visit(const Logical *) {}
void CodeGen::visit(const Call *) {}
void CodeGen::visit(const Get *) {}
void CodeGen::visit(const Set *) {}
void CodeGen::visit(const GetIndex *) {}
void CodeGen::visit(const SetIndex *) {}
void CodeGen::visit(const This *) {}
