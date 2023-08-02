#ifndef _NL_BACKENDS_X86_64_CODEGEN_H_
#define _NL_BACKENDS_X86_64_CODEGEN_H_

#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "expr-types.h"
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
public:
  using ValueRef = std::string;
  using Register = std::string;

  // Assign the expression to a value holder
  // on the machine.
  ValueRef makeAssignable(const Expr* expr) {
    // Is it already assignable?
    auto it = exprToRegister_.find(expr);
    if (it != exprToRegister_.end()) {
      return it->second;
    }
    if (!unusedGpRegs_.empty()) {
      auto const it = unusedGpRegs_.begin();
      auto const reg = *it;
      exprToRegister_[expr] = reg;
      registerToExpr_[reg] = expr;
      unusedGpRegs_.erase(it);
      return reg;
    }
    assert(false && "No GP regs to assign");
  }

  void overwrite(const Expr* expr, const ValueRef& v) {
    if (v[0] == '%') { regOverwrite(expr, v); return; }
    exprToRef_[expr] = v;
  }

  void regOverwrite(const Expr *expr, const Register &reg) {
      exprToRef_[expr] = reg;
      exprToRegister_[expr] = reg;
      registerToExpr_[reg] = expr;
  }

  void assign(const Expr *expr, const std::string &immediate) {
    exprToRef_[expr] = immediate;
  }

  ValueRef get(const Expr *expr) {
    auto it = exprToRef_.find(expr);
    assert(it != exprToRef_.end() && "ValueRef requested for unknown expr");
    return it->second;
  }


  // Ensure no expressions use the register,
  // spilling them to memory if necessary.
  // void regObtain(const Register&);

  void regFree(const Register& reg) {
    auto it = registerToExpr_.find(reg);
    if (it == registerToExpr_.end()) { return; }
    auto expr = it->second;
    registerToExpr_.erase(it);
    exprToRef_.erase(expr);
    exprToRegister_.erase(expr);
    unusedGpRegs_.insert(reg);
  }

private:
  std::unordered_set<Register> unusedGpRegs_ { "%r10", "%r11" , "%r12"};
  std::unordered_map<const Expr*, std::string> exprToRef_;
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
  CodeGen(const ExprTypes &exprTypes) : exprTypes_(exprTypes) {}
  virtual void generate(const std::vector<Stmt *> &program);
  void dump() const;

  OVERRIDE_EXPR_VISITOR_FNS(void)
  OVERRIDE_STMT_VISITOR_FNS(void)
private:
  void emit(const std::vector<Stmt *> &stmts);
  void emit(const Stmt *stmt);
  void emit(const Expr *expr);

  ValueRefTracker valueRefs_;

  const ExprTypes &exprTypes_;

  Section rodata_;
  Section data_;
  Section text_;

  // struct Stats { uint32_t memReads, memWrites, regReads, regWrites; };
  // Stats stats_;
};
} // namespace x86_64

#endif // _NL_BACKENDS_X86_64_CODEGEN_H_
