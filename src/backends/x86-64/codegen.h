#ifndef _NL_BACKENDS_X86_64_CODEGEN_H_
#define _NL_BACKENDS_X86_64_CODEGEN_H_

#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "backends/abstract-codegen.h"
#include "visitor.h"

namespace x86_64 {

// Named x86_64 but we try to keep most logic other than
// instruction-selection as machine-agnostic as possible.

// A reference to a logical value in the target machine
// assembly language. Can be a register, memory location,
// immediate etc.
// Question: Should we get rid of immediate here, and make
// it an optimization pass?

// A naive implementation of register allocation which falls
// back to storing values on the stack.
class ValueRefTracker {
  using ValueRef = std::string;
  using Register = std::string;

  // Assign the expression to a value holder
  // on the machine.
  ValueRef assign(const Expr*);

  // Ensure no expressions use the register,
  // spilling them to memory if necessary.
  void freeRegister(const Register&);

  std::unordered_map<const Expr*, Register> exprToRegister_;
  std::unordered_map<Register, const Expr*> registerToExpr_;
  uint8_t stackOffset;
};

struct AsmLine {
  enum class Kind { Label, Instruction, Directive };
  Kind kind;
  std::vector<std::string> values;
  bool isLabel() const { return kind == Kind::Label; }
};

struct Section {
  void instr(std::vector<std::string> &&v) {
    push(AsmLine::Kind::Instruction, std::move(v));
  }
  void label(std::vector<std::string> &&v) {
    push(AsmLine::Kind::Label, std::move(v));
  }
  void directive(std::vector<std::string> &&v) {
    push(AsmLine::Kind::Directive, std::move(v));
  }
  void push(AsmLine::Kind kind, std::vector<std::string> &&v) {
    contents.push_back({kind, std::move(v)});
  }

  std::vector<AsmLine> contents;
};

class CodeGen : public AbstractCodegen,
                public ExprVisitor<>,
                public StmtVisitor<> {
public:
  CodeGen() {}
  virtual void generate(const std::vector<Stmt *> &program);
  void dump() const;

  OVERRIDE_EXPR_VISITOR_FNS(void)
  OVERRIDE_STMT_VISITOR_FNS(void)
private:
  void emit(const std::vector<Stmt *> &stmts);
  void emit(const Stmt *stmt);
  void emit(const Expr *expr);

  Section rodata_;
  Section data_;
  Section text_;

  // struct Stats { uint32_t memReads, memWrites, regReads, regWrites; };
  // Stats stats_;
};
} // namespace x86_64

#endif // _NL_BACKENDS_X86_64_CODEGEN_H_
