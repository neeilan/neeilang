#ifndef _NL_TYPE_CHECKER_H_
#define _NL_TYPE_CHECKER_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "expr.h"
#include "scope_manager.h"
#include "stmt.h"
#include "symtab.h"
#include "type_table.h"
#include "visitor.h"

class TypeChecker : public ExprVisitor<void>, public StmtVisitor<void> {
public:
  TypeChecker(ScopeManager &sm) : sm(sm) {}

  void check(const std::vector<Stmt *> stmts);
  void check(const Stmt *stmt);
  NLType check(const Expr *expr);

  EXPR_VISITOR_METHODS(void)
  STMT_VISITOR_METHODS(void)

  bool match(const NLType type, const std::vector<NLType> &types);
  bool match(const Expr *expr, const std::vector<NLType> &types);
  bool has_type_error(const std::vector<NLType> &types);

  std::shared_ptr<TypeTable> types() { return sm.current().typetab; }
  std::shared_ptr<SymbolTable> symbols() { return sm.current().symtab; }
  std::map<const Expr *, NLType> get_expr_types() { return expr_types; }

  ScopeManager &sm;

private:
  std::map<const Expr *, NLType> expr_types;
  NLType enclosing_class;
  std::shared_ptr<FuncType> enclosing_fn;
};

#endif //_NL_TYPE_CHECKER_H_
