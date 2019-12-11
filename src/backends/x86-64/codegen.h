#ifndef _NL_BACKENDS_X86_64_CODEGEN_H_
#define _NL_BACKENDS_X86_64_CODEGEN_H_

#include <fstream>
#include <ostream>
#include <sstream>
#include <string>

#include "backends/abstract-codegen.h"
#include "visitor.h"

#include "backends/x86-64/address.h"
#include "backends/x86-64/asm.h"
#include "backends/x86-64/label.h"
#include "backends/x86-64/register.h"
#include "backends/x86-64/syscall.h"

namespace X86_64 {
class CodeGen : public AbstractCodegen,
                public ExprVisitor<>,
                public StmtVisitor<> {
public:
  CodeGen();
  void exit(const int status);
  void dump(std::ostream &out);
  Address str_const(const std::string &s);
  void print(const Address &addr);
  void begin_func(const std::string &name);
  const Register &add(const Register &a, const Register &b);
  const Register &move(const Register &dest, const Register &src);
  const Register load_const(int value);
  void call(const std::string &label);
  void ret();
  void print(const Register &r);

  void push(const Register &r);
  void pop(const Register &r);
  const Register pop();

  EXPR_VISITOR_METHODS(void)
  STMT_VISITOR_METHODS(void)

  virtual void generate(const std::vector<Stmt *> &program);

private:
  Asm_t *asm_tail = NULL;
  void asm_emit(std::vector<const char *> args);
  std::ostringstream _asm;
  std::ostringstream _data;
  std::ostringstream _bss;
  std::ostringstream &_instr();

  void emit(const std::vector<Stmt *> &stmts);
  void emit(const Stmt *stmt);
  void emit(const Expr *expr);
};
} // namespace X86_64

#endif // _NL_BACKENDS_X86_64_CODEGEN_H_
