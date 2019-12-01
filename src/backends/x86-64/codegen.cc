#include "backends/x86-64/codegen.h"
#include "backends/x86-64/asm.h"

using CodeGen = X86_64::CodeGen;

void CodeGen::generate(const std::vector<Stmt *> &program) { emit(program); }

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
void CodeGen::visit(const Binary *) {}
void CodeGen::visit(const Grouping *) {}
void CodeGen::visit(const StrLiteral *) {}

void CodeGen::visit(const NumLiteral *expr) {
  asm_tail = asm_create_1arg(asm_tail, "push", expr->value.c_str());
  asm_print(stderr, asm_tail);
}

void CodeGen::visit(const BoolLiteral *) {}
void CodeGen::visit(const Variable *) {}
void CodeGen::visit(const Assignment *) {}
void CodeGen::visit(const Logical *) {}
void CodeGen::visit(const Call *) {}
void CodeGen::visit(const Get *) {}
void CodeGen::visit(const Set *) {}
void CodeGen::visit(const GetIndex *) {}
void CodeGen::visit(const SetIndex *) {}
void CodeGen::visit(const This *) {}
