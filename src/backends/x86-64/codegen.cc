#include "backends/x86-64/codegen.h"

#include <iostream>

#include "primitives.h"

namespace x86_64 {

void CodeGen::generate(const std::vector<Stmt *> &program) {
  // Setup format strings for printf
  rodata_.directive({"format_printf_int: .asciz \"%d\\n\""});
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
  std::cerr << "[PrintStmt]" << stmt->expression << std::endl;
  auto const* e = stmt->expression;
  if (!e) {
    return;
  }
  emit(e);

  auto const exprType = exprTypes_.find(e);
  if (exprType->second == Primitives::String()) {
    // TODO(neeilan): Does this let us print a variable that's a string?
    // Need memory references to be rip-relative to produce position independent executables
    // i.e we want the assembler to emit a RIP-relative relocation rather than an absolute
    // R_X86_64_32, since gcc invokes the linker in PIE mode by default.
    // Same for printf format strings below.
    text_.instr({"lea", valueRefs_.get(e) + "(%rip)", "%rdi"});
    text_.instr({"call", "puts"});
    return;
  }

  // %rdi and %rsi hold first two integer/pointer function params
  // per x86-64 System V calling convention
  auto const src = valueRefs_.get(e);
  text_.instr({"lea", "format_printf_int(%rip)", "%rdi"});
  text_.instr({"mov", src, "%rsi"});
  text_.instr({"call", "printf"});
  valueRefs_.regFree(src); // what if print(x) - we need to know when to free
}
void CodeGen::visit(const VarStmt *stmt) {
  std::cerr << "[VarStmt]" << std::endl;
}
void CodeGen::visit(const ClassStmt *stmt) {
  std::cerr << "[ClassStmt]" << std::endl;
}
void CodeGen::visit(const IfStmt *stmt) {
  static uint16_t id = 1;
  emit(stmt->condition);
  const auto ref = valueRefs_.get(stmt->condition);
  // TODO: Is is fine if cond is memory and not register?
  auto const cond = valueRefs_.makeAssignable(stmt->condition);
  if (ref != cond) {
    text_.instr({"mov", ref, cond });
  }

  auto elseLabel = std::string("_post_if_") + std::to_string(id++);
  auto postElseLabel = std::string("_post_else_") + std::to_string(id);
  // cond AND cond
  // If result is 0, set zero flag to 1
  // i.e. if cond is false, set ZF to 1
  text_.instr({"test", cond, cond });
  // je = jump if ZF=1 (i.e cond is 0)
  text_.instr({"je", elseLabel});
  emit(stmt->then_branch);
  if (stmt->else_branch) {
    text_.instr({"jmp", postElseLabel});
  }
  text_.label({elseLabel});
  if (stmt->else_branch) {
    emit(stmt->else_branch);
    text_.label({postElseLabel});
  }
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
  // We can't add into a literal, so we need an 'assignable' dest
  auto const dest = valueRefs_.makeAssignable(&expr->left);
  std::cerr << "[dest=" << dest << "]" << std::endl;
  valueRefs_.regOverwrite(&expr->left, dest);
  if (left != dest) {
    text_.instr({"mov", left, dest });
  }

  auto binaryOpEmit = [&](auto const &opcode){
    // We use `right` as the first operand because
    // sub behaves as dest-=right
    text_.instr({ opcode, right, dest });
    valueRefs_.regOverwrite(expr, dest);
    // Can resuse `right` as dest is the accumulator
    valueRefs_.regFree(right);
  };

  switch (expr->op.type) {
  // For the following instructions, we know both operands are Ints or Floats
  // (since we don't handle String concat with '+' yet).
  case PLUS: {
    binaryOpEmit("add");
    break;
  }
  case MINUS: {
    binaryOpEmit("sub");
    break;
  }
  case STAR: {
    binaryOpEmit("imul");
    break;
  }
  default: { std::cerr << "[Unimplemented BinaryOp]" << std::endl; }
  }
}

void CodeGen::visit(const Grouping *expr) {
  auto const *e = &expr->expression;
  emit(e);
  valueRefs_.overwrite(expr, valueRefs_.get(e));
}

void CodeGen::visit(const StrLiteral *expr) {
  static uint16_t strLiteralId = 1;
  auto const label = std::string("_str_literal_") + std::to_string(strLiteralId++);
  rodata_.directive({label + ": .asciz \"" + expr->value + "\""});
  valueRefs_.assign(expr, label);
}

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

void CodeGen::visit(const BoolLiteral *expr) {
  const auto immediate = expr->value ? "$1" : "$0";
  valueRefs_.assign(expr, immediate);
}

void CodeGen::visit(const Variable *) {}
void CodeGen::visit(const Assignment *) {}

void CodeGen::visit(const Logical *expr) {
  emit(&expr->left);
  emit(&expr->right);

  auto const left = valueRefs_.get(&expr->left);
  auto const right = valueRefs_.get(&expr->right);
  auto const dest = valueRefs_.makeAssignable(&expr->left);

  auto binaryOpEmit = [&](auto const &opcode) {
    text_.instr({opcode, right, dest});
    valueRefs_.regOverwrite(expr, dest);
    // Can resuse `right` as dest is the accumulator
    valueRefs_.regFree(right);
  };

  switch (expr->op.type) {
    case AND: {
    binaryOpEmit("and");
    return;
    }
    case OR: {
    binaryOpEmit("or");
    return;
    }
    default: {
    return;
    }
  }
}

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
