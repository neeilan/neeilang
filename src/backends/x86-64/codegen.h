#ifndef _NL_BACKENDS_X86_64_CODEGEN_H_
#define _NL_BACKENDS_X86_64_CODEGEN_H_

#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "ast-printer.h"
#include "expr-types.h"
#include "scope-manager.h"
#include "backends/abstract-codegen.h"
#include "stackframe.h"
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
    if (it != exprToRegister_.end() && it->second != "%rax") {
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

    AstPrinter ap;
    std::cerr << "[makeAssignable] No GP register to assign for expr [" << ap.print(expr) << "]\n"
      << "  Current assignments:\n";
    for (auto it = registerToExpr_.begin(); it != registerToExpr_.end(); ++it) {
      std::cerr << "   > " << it->first << " : [" << ap.print(it->second) << "]\n";
    }
    exit(1);
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

  void assign(const Expr *expr, const std::string &immediateOrLabel) {
    exprToRef_[expr] = immediateOrLabel;
  }

  ValueRef get(const Expr *expr) {
    auto it = exprToRef_.find(expr);
    if (it == exprToRef_.end()) {
      AstPrinter ap;
      std::cerr << "ValueRef requested for unknown expr [" << ap.print(expr) << "]" << std::endl;
      exit(1);
    }
    return it->second;
  }

  std::pair<std::string, bool> acquireRegister(const Expr* expr) {
    // TODO: returns register. and whether it need to be restored from stack. Needed because we can't to stuff like memory-to-memory `mov`s
    // Is it already assignable?
    auto it = exprToRegister_.find(expr);
    if (it != exprToRegister_.end()) {
      return {it->second, false};
    }
    if (!unusedGpRegs_.empty()) {
      auto const it = unusedGpRegs_.begin();
      auto const reg = *it;
      exprToRegister_[expr] = reg;
      registerToExpr_[reg] = expr;
      unusedGpRegs_.erase(it);
      return {reg, false};
    }

    AstPrinter ap;
    std::cerr << "[acquireRegister] No GP register to assign for expr [" << ap.print(expr) << "]\n"
      << "  Current assignments:\n";
    for (auto it = registerToExpr_.begin(); it != registerToExpr_.end(); ++it) {
      std::cerr << "   > " << it->first << " : [" << ap.print(it->second) << "]\n";
    }
    exit(1);
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

  void resetRegisters() {
    for (auto const & [reg, _] : registerToExpr_) {
      unusedGpRegs_.insert(reg);
    }
    for (auto const & reg : unusedGpRegs_) {
      regFree(reg);
    }
  }

private:
  std::unordered_set<Register> unusedGpRegs_ { "%r10", "%r11" , "%r12", "%r13"};
  std::unordered_map<const Expr*, std::string> exprToRef_;
  std::unordered_map<const Expr*, Register> exprToRegister_;
  std::unordered_map<Register, const Expr*> registerToExpr_;
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
  CodeGen(const ExprTypes &exprTypes, ScopeManager &sm) : exprTypes_(exprTypes), sm_(sm)
  , stackFrames_(StackFrameSizer(sm))
  {}
  virtual void generate(const std::vector<Stmt *> &program) override;
  void dump() const;

  OVERRIDE_EXPR_VISITOR_FNS(void)
  OVERRIDE_STMT_VISITOR_FNS(void)
private:
  void emit(const std::vector<Stmt *> &stmts);
  void emit(const Stmt *stmt);
  void emit(const Expr *expr);

  void enterScope() { sm_.enter(); namedVals = std::make_shared<CactusTable<std::string, ValueRefTracker::ValueRef>>(namedVals); };
  void exitScope() { sm_.exit(); assert(namedVals && "Only scope on stack!"); namedVals = namedVals->parent; };
  std::shared_ptr<CactusTable<std::string, ValueRefTracker::ValueRef>> namedVals;



  ValueRefTracker::ValueRef emitArrayInit(NLType nlType, const std::vector<const Expr *>& dims);
  ValueRefTracker::ValueRef emitClassInit(NLType nlType);

  ValueRefTracker valueRefs_;

  const ExprTypes &exprTypes_;
  ScopeManager &sm_;
  std::unordered_set<std::string> funcLabels_;
  std::vector<ClassStmt const*> classes_;

  StackFrameSizer stackFrames_;
  const FuncStmt * enclosingFunc_ = nullptr;
  NLType enclosingClass_ = nullptr;
  ValueRefTracker::ValueRef lastDereferencedObj_;


  Section rodata_;
  Section data_;
  Section text_;

  // struct Stats { uint32_t memReads, memWrites, regReads, regWrites; };
  // Stats stats_;
};
} // namespace x86_64

#endif // _NL_BACKENDS_X86_64_CODEGEN_H_
