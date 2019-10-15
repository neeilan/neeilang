#ifndef _NL_TYPE_CHECKER_H_
#define _NL_TYPE_CHECKER_H_

#include <string>
#include <map>
#include <memory>
#include <vector>

#include "expr.h"
#include "stmt.h"
#include "type_table.h"
#include "visitor.h"

class TypeChecker :
  public ExprVisitor<void>,
  public StmtVisitor<void>
{
public:
    TypeChecker(TypeTable & types) : types(types) {}

    void check(const std::vector<Stmt *> stmts);
    void check(const Stmt * stmt);
    std::shared_ptr<Type> check(const Expr * expr);
    
    void visit(const Binary *);
    void visit(const StrLiteral *);
    void visit(const NumLiteral *);
    void visit(const BoolLiteral *);
    void visit(const Grouping *);
    void visit(const Unary *);
    void visit(const Variable *);
    void visit(const Assignment *);
    void visit(const Logical *);
    void visit(const Call *);
    void visit(const Get *);
    void visit(const Set *);
    void visit(const This *);

    void visit(const BlockStmt *);
    void visit(const ExprStmt *);
    void visit(const PrintStmt *);
    void visit(const VarStmt *);
    void visit(const ClassStmt *);
    void visit(const IfStmt *);
    void visit(const WhileStmt *);
    void visit(const FuncStmt *);
    void visit(const ReturnStmt *);

    bool match(const std::shared_ptr<Type> type, const std::vector<std::shared_ptr<Type>> & types);
    bool match(const Expr * expr, const std::vector<std::shared_ptr<Type>> & types);
    bool has_type_error(const std::vector<std::shared_ptr<Type>> & types);

    TypeTable & types;

private:
    std::shared_ptr<Type> enclosing_class;
    std::shared_ptr<Type> enclosing_func;
};

#endif //_NL_TYPE_CHECKER_H_
