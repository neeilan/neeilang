#include "backends/x86-64/codegen.h"

#include <iostream>

#include "primitives.h"

namespace x86_64 {

void CodeGen::generate(const std::vector<Stmt *> &program) {
  text_.directive({".global main"});
  emit(program);
}

void CodeGen::emit(const std::vector<Stmt *> &stmts) {
  for (const Stmt *stmt : stmts)
    emit(stmt);
}

void CodeGen::emit(const Stmt *stmt) { stmt->accept(this); }
void CodeGen::emit(const Expr *expr) { expr->accept(this); }

void CodeGen::visit(const ExprStmt *stmt) { emit(stmt->expression); }
void CodeGen::visit(const BlockStmt *stmt) {
  std::cerr << "[BlockStmt]" << std::endl;
  emit(stmt->block_contents);
}
void CodeGen::visit(const PrintStmt *stmt) {
  std::cerr << "[PrintStmt]" << std::endl;
}
void CodeGen::visit(const VarStmt *stmt) {
  std::cerr << "[VarStmt]" << std::endl;
}
void CodeGen::visit(const ClassStmt *stmt) {
  std::cerr << "[ClassStmt]" << std::endl;
}
void CodeGen::visit(const IfStmt *stmt) {
  std::cerr << "[IfStmt]" << std::endl;
}
void CodeGen::visit(const WhileStmt *stmt) {
  std::cerr << "[WhileStmt]" << std::endl;
}

void CodeGen::visit(const FuncStmt *stmt) {
  text_.label({stmt->name.lexeme});
  text_.instr({"push", "%rbx"});
  emit(stmt->body);
}

void CodeGen::visit(const ReturnStmt *stmt) {
  std::cerr << "[ReturnStmt]" << std::endl;
  if (stmt->value) {
    emit(stmt->value);
  }

  text_.instr({"mov", valueRefs_.get(stmt->value), "%rax"});
  text_.instr({"pop", "%rbx"});
  text_.instr({"ret"});
}

void CodeGen::visit(const Unary *) {}

void CodeGen::visit(const Binary *expr) {
  std::cerr << "[BinaryOp]" << std::endl;
  // TODO(neeilan): For short-circuiting, wait to emit right
  // TODO(neeilan): Explore passing left register to accumulate
  emit(&expr->left);
  emit(&expr->right);
  std::cerr << "[BinaryOp - L/R emitted]" << std::endl;

  auto const left = valueRefs_.get(&expr->left);
  auto const right = valueRefs_.get(&expr->right);

  std::cerr << "[BinaryOp - L/R refs found: L=" << left << " R=" << right << "]" << std::endl;
  // We can't add into a literal, so we need an 'assignable' value ref
  // probably want a function like ValueRef ensureAssignable(valueRef)
  auto const rightAssignable = valueRefs_.assign(&expr->right);
  std::cerr << "[rightAssignable=" << rightAssignable << "]" << std::endl;
  valueRefs_.regOverwrite(&expr->right, rightAssignable);
  text_.instr({"mov", right, rightAssignable });

  switch (expr->op.type) {
  // For the following instructions, we know both operands are Ints or Floats
  // (since we don't handle String concat with '+' yet).
  case PLUS: {
  // add src, dest
  text_.instr({"add", left, rightAssignable });
  valueRefs_.regOverwrite(expr, rightAssignable);
  break;
  }
  case MINUS: {
    text_.instr({"sub", left, rightAssignable });
    valueRefs_.regOverwrite(expr, rightAssignable);
  }
  default: { std::cerr << "[Unimplemented BinaryOp]" << std::endl; }
  }
}

void CodeGen::visit(const Grouping *expr) { emit(&expr->expression); }
void CodeGen::visit(const StrLiteral *) {}

void CodeGen::visit(const NumLiteral *expr) {
  std::cerr << "[NumLiteral]" << std::endl;
  auto const exprType = exprTypes_.find(expr);
  if(exprType == exprTypes_.end()) { std::cerr << "[Unknown ExprType]" << std::endl; return; }
  if (exprType->second == Primitives::Float()) {
    std::cerr << "[Float Literal]" << std::endl;
  } else if (exprType->second == Primitives::Int()) {
    // e.g. 5 becomes $5
    valueRefs_.assign(expr, "$" + expr->value);
  }
  
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
void CodeGen::visit(const SentinelExpr *) {}

void CodeGen::dump() const {
  std::stringstream ss;
  auto dumpLine = [&](auto const &asmLine) {
    auto const &line = asmLine.values;
    if (asmLine.isLabel()) {
      ss << line[0] << ":\n";
      return;
    }
    ss << ' ' << line[0];
    for (size_t i = 1; i < line.size() - 1; ++i) {
      ss << ' ' << line[i] << ", ";
    }
    if (line.size() > 1) {
      ss << ' ' << line.back();
    }
    ss << '\n';
  };
  auto dumpSection = [&](auto const &name, auto const &s) {
    ss << name << '\n';
    for (auto const &l : s.contents) {
      dumpLine(l);
    }
  };
  dumpSection(".section .rodata", rodata_);
  dumpSection(".data", data_);
  dumpSection(".text", text_);
  std::cout << ss.str();
}

} // namespace x86_64
