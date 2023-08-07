#include "stackframe.h"

namespace x86_64 {

void StackFrameSizer::init(const std::vector<Stmt *> &program) {
  for (const Stmt *stmt : program) {
    init(stmt);
  }
}

void StackFrameSizer::init(const Stmt *stmt) { stmt->accept(this); }

void StackFrameSizer::visit(const FuncStmt *stmt) {
  sm_.enter();
  auto *oldEnclosing = enclosingFunc;
  enclosingFunc = stmt;
  // Do any parameters here
  // Body
  init(stmt->body);
  enclosingFunc = oldEnclosing;
  sm_.exit();
}

void StackFrameSizer::visit(const VarStmt *stmt) {
  NLType nlType = sm_.current().typetab->get(stmt->name.lexeme);

  auto nlTypeTox86TypeSize = [](auto nlType) {
    if (nlType == Primitives::Int()) {
      return 8;
    }
    return 8;
  };
  bases[enclosingFunc].addLocal(stmt, nlTypeTox86TypeSize(nlType));
}

void StackFrameSizer::visit(const BlockStmt *stmt) {
  sm_.enter();
  init(stmt->block_contents);
  sm_.exit();
}

void StackFrameSizer::visit(const ClassStmt *stmt) {
  for (const Stmt *method : stmt->methods) {
    init(method);
  }
}

void StackFrameSizer::visit(const IfStmt *stmt) {
  init(stmt->then_branch);
  if (stmt->else_branch) {
    init(stmt->else_branch);
  }
}

void StackFrameSizer::visit(const WhileStmt *stmt) {
  if (stmt->body) {
    init(stmt->body);
  }
}

// These can't hold VarStmts
void StackFrameSizer::visit(const ReturnStmt *) {}
void StackFrameSizer::visit(const ExprStmt *) {}
void StackFrameSizer::visit(const PrintStmt *) {}

}  // namespace x86_64