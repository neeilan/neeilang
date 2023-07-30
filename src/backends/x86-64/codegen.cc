#include "backends/x86-64/codegen.h"

#include <iostream>

namespace x86_64 {

void CodeGen::generate(const std::vector<Stmt *> &program) {
  emit(program);
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
void CodeGen::visit(const Binary *) {}
void CodeGen::visit(const Grouping *expr) { emit(&expr->expression); }
void CodeGen::visit(const StrLiteral *) {}
void CodeGen::visit(const NumLiteral *) {}
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
void CodeGen::visit(const SentinelExpr *) {}

void CodeGen::dump() const {
  std::stringstream ss;
  auto dumpLine = [&](auto const &asmLine) {
    auto const &line = asmLine.values;
    if (asmLine.isLabel()) {
      ss << line[0] << ":\n";
      return;
    }
    ss << line[0];
    for (size_t i = 1; i < line.size() - 1; ++i) {
      ss << line[i] << ", ";
    }
    if (line.size() > 1) {
      ss << line.back();
    }
    ss << '\n';
  };
  auto dumpSection = [&](auto const &name, auto const &contents) {
    ss << name << '\n';
    for (auto const &l : contents) {
      dumpLine(l);
    }
  };
  dumpSection(".rodata", rodata_);
  dumpSection(".data", data_);
  dumpSection(".text", text_);
  std::cout << ss.str();
}

} // namespace x86_64
