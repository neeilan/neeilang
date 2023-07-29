#ifndef _NL_BACKENDS_X86_64_CODEGEN_H_
#define _NL_BACKENDS_X86_64_CODEGEN_H_

#include <fstream>
#include <ostream>
#include <sstream>
#include <string>

#include "backends/abstract-codegen.h"
#include "visitor.h"

namespace X86_64 {
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
};
} // namespace X86_64

#endif // _NL_BACKENDS_X86_64_CODEGEN_H_
