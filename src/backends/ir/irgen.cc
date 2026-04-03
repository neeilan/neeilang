#include "irgen.h"

#include <stdexcept>

std::ostream& IRGen::nextReg() {
    return ir_ << "%" << (nextReg_++) << " = ";
}

void IRGen::generate(const std::vector<const Stmt *> &program) {
  ir_ << "############\n";
  ir_ << "# IR dump\n";
  ir_ << "############\n\n";
  emit(program);
}

void IRGen::emit(const std::vector<const Stmt *> &stmts) {
  for (const Stmt *stmt : stmts) {
    emit(stmt);
  }
}

void IRGen::emit(const Stmt *stmt) { stmt->accept(this); }

void IRGen::emit(const Expr *expr) {
  expr->accept(this);
}

void IRGen::visit(const Unary *expr) {
  assert(false && "Unimplemented");
}

void IRGen::visit(const Binary *expr) {
    emit(&expr->left);
    emit(&expr->right);

    auto rL = reg_[&expr->left];
    auto rR = reg_[&expr->right];

    reg_[expr] = nextReg_;
    switch (expr->op.type) {
        case PLUS: {
            nextReg() << "add %" << rL << " %" << rR << "\n";
        default:
            break;
        }

    }
}

void IRGen::visit(const NumLiteral *expr) {
  reg_[expr] = nextReg_;
  nextReg() << expr->value << "\n";
}

void IRGen::visit(const Grouping *expr) {
  assert(false && "Unimplemented");
}

void IRGen::visit(const StrLiteral *expr) {
  assert(false && "Unimplemented");
}

void IRGen::visit(const BoolLiteral *expr) {
  assert(false && "Unimplemented");
}

void IRGen::visit(const Logical *expr) {
  assert(false && "Unimplemented");
}

void IRGen::visit(const VarStmt *stmt) {
  assert(false && "Unimplemented");
}

void IRGen::visit(const Variable *expr) {
  assert(false && "Unimplemented");
}

void IRGen::visit(const Assignment *expr) {
  assert(false && "Unimplemented");
}

void IRGen::visit(const Call *expr) {
  assert(false && "Unimplemented");
}

void IRGen::visit(const Get *expr) {
  assert(false && "Unimplemented");
}

void IRGen::visit(const Set *expr) {
  assert(false && "Unimplemented");
}

void IRGen::visit(const This *expr) {
  assert(false && "Unimplemented");
}

void IRGen::visit(const ExprStmt *stmt) { emit(stmt->expression); }

void IRGen::visit(const BlockStmt *stmt) {
  emit(stmt->block_contents);
  ir_ << "\n";
}

void IRGen::visit(const IfStmt *stmt) {
  assert(false && "Unimplemented");
}

void IRGen::visit(const PrintStmt *stmt) {
  // Print
}

void IRGen::visit(const NamespaceStmt *stmt) {
  emit(stmt->contents);
}

void IRGen::visit(const ScopedEnum *) {
    // noop
}

void IRGen::visit(const TemplateStmt *) {
    // noop
}

void IRGen::visit(const StaticAssertStmt *) {
    // noop
}

void IRGen::visit(const UsingStmt *) {
    // noop
}

void IRGen::visit(const AliasStmt *) {
    // noop
}

void IRGen::visit(const ClassStmt *stmt) {
  assert(false && "Unimplemented");
}

void IRGen::visit(const ExplicitClassTemplateInitialization *stmt) {
  assert(false && "Unimplemented");
}

void IRGen::visit(const WhileStmt *stmt) {
  assert(false && "Unimplemented");
}

void IRGen::visit(const FuncStmt *stmt) {
  ir_ << stmt->ctx->name() << "::" << stmt->name.lexeme << ":\n";
  for (auto s : stmt->body) {
    emit(s);
  }
}

void IRGen::visit(const ReturnStmt *stmt) {
  emit(stmt->value);
  ir_ << "ret %" << reg_[stmt->value] << "\n";
}

void IRGen::visit(const GetIndex *expr) {
  assert(false && "Unimplemented");
}

void IRGen::visit(const SetIndex *expr) {
  assert(false && "Unimplemented");
}

void IRGen::visit(const SizeOf *) {
  assert(false && "Unimplemented");
}
void IRGen::visit(const AlignOf *) {
  assert(false && "Unimplemented");
}

void IRGen::visit(const StaticCast *) {
  assert(false && "Unimplemented");
}

void IRGen::visit(const SentinelExpr *expr) {
  // noop
}
