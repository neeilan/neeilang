#ifndef _NL_AST_PRINTER_H_
#define _NL_AST_PRINTER_H_

#include <string>

#include "expr.h"
#include "stmt.h"
#include "visitor.h"

class AstPrinter :
  public ExprVisitor<std::string>,
  public StmtVisitor<std::string>
{
  public:
    std::string print(const Expr & expr)
    {
        return expr.accept(this);
    }

    std::string print(const Stmt & stmt)
    {
        return stmt.accept(this);
    }

    std::string visit(const Unary *);
    std::string visit(const Binary *);
    std::string visit(const Grouping *);
    std::string visit(const StrLiteral *);
    std::string visit(const NumLiteral *);
    std::string visit(const BoolLiteral *);
    std::string visit(const Variable *);
    std::string visit(const Assignment *);
    std::string visit(const Logical *);
    std::string visit(const Call *);
    std::string visit(const Get *);
    std::string visit(const Set *);
    std::string visit(const This *);

    std::string parenthesize(std::string, const Expr *);
    std::string parenthesize(std::string, const Expr *, const Expr *);

    std::string visit(const BlockStmt*);
    std::string visit(const ExprStmt*);
    std::string visit(const PrintStmt*);
    std::string visit(const VarStmt*);
    std::string visit(const ClassStmt*);
    std::string visit(const IfStmt*);
    std::string visit(const WhileStmt*);
    std::string visit(const FuncStmt*);
    std::string visit(const ReturnStmt*);
};

#endif //_NL_AST_PRINTER_H_
