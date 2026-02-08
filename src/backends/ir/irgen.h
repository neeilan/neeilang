#pragma once

#include "backends/abstract-codegen.h"

#include "ir-instr.h"

#include <iostream>
#include <sstream>
#include <unordered_map>

class IRGen : public AbstractCodegen,
              public ExprVisitor<>,
              public StmtVisitor<> {

public:
  OVERRIDE_EXPR_VISITOR_FNS(void)
  OVERRIDE_STMT_VISITOR_FNS(void)

  virtual void generate(const std::vector<const Stmt *> &program) override;
  void dump() { std::cout <<  ir_.str() << std::endl; }

private:
  void emit(const std::vector<const Stmt *> &stmts);
  void emit(const Stmt *stmt);
  void emit(const Expr *expr);

  std::stringstream ir_;
  std::unordered_map<Expr const*, uint32_t> reg_;
  uint32_t nextReg_ = 0;
  std::ostream& nextReg();

};
