#ifndef _NL_AST_PRINTER_H_
#define _NL_AST_PRINTER_H_

#include <string>
#include <vector>

#include "expr.h"
#include "stmt.h"
#include "visitor.h"

class AstPrinter : public ExprVisitor<std::string>,
                   public StmtVisitor<std::string> {
public:
  std::string print(const std::vector<Stmt*> & program);
  std::string print(const Expr * expr) { return expr->accept(this); }
  std::string print(const Stmt * stmt) { return stmt->accept(this); }

  EXPR_VISITOR_METHODS(std::string)
  STMT_VISITOR_METHODS(std::string)

  std::string parenthesize(std::string, const Expr *);
  std::string parenthesize(std::string, const Expr *, const Expr *);
};

#endif //_NL_AST_PRINTER_H_
