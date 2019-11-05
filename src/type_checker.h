#ifndef _NL_TYPE_CHECKER_H_
#define _NL_TYPE_CHECKER_H_

#include <string>
#include <map>
#include <memory>
#include <vector>

#include "expr.h"
#include "stmt.h"
#include "symtab.h"
#include "type_table.h"
#include "visitor.h"
#include "scope_manager.h"

class TypeChecker : public ExprVisitor<void>, public StmtVisitor<void> {
public:
  TypeChecker(ScopeManager &sm) : sm(sm) {}

  void check(const std::vector<Stmt *> stmts);
  void check(const Stmt *stmt);
  std::shared_ptr<Type> check(const Expr *expr);

  EXPR_VISITOR_METHODS(void)
  STMT_VISITOR_METHODS(void)

  bool match(const std::shared_ptr<Type> type,
             const std::vector<std::shared_ptr<Type>> &types);
  bool match(const Expr *expr, const std::vector<std::shared_ptr<Type>> &types);
  bool has_type_error(const std::vector<std::shared_ptr<Type>> &types);

  std::shared_ptr<TypeTable> types() { return sm.current().typetab; }

  std::shared_ptr<SymbolTable> symbols() { return sm.current().symtab; }
  std::map<const Expr *, std::shared_ptr<Type>> get_expr_types() { return expr_types; }
  
  ScopeManager &sm;

private:
  std::map<const Expr *, std::shared_ptr<Type>> expr_types;
  std::shared_ptr<Type> enclosing_class;
  std::shared_ptr<Type> enclosing_fn;
};

#endif //_NL_TYPE_CHECKER_H_
