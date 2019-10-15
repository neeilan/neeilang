#ifndef _NL_RESOLVER_H_
#define _NL_RESOLVER_H_

#include <string>
#include <map>
#include <vector>

#include "token.h"
#include "visitable.h"
#include "visitor.h"
#include "scope_tracker.h"

// Describes what kind of function (if any) we're resolving
// names in.
enum FunctionType
{
    NOT_IN_FN,
    METHOD,
    FUNCTION,
    INITIALIZER,
};

// Describes whether we're currently resolving names
// within in a class. Keywords like 'this' only make
// sense within a class.
enum ClassType
{
    NOT_IN_CLASS,
    IN_CLASS
};

// true in map == 'is finished being initialized in this scope'
struct ScopeMap {
  std::map<std::string, bool> map;
  size_t id = 0;
};

class Resolver : public ExprVisitor<void>, public StmtVisitor<void>
{
  public:
    Resolver() {
      scopes.push_back(&globals);
    }

    void resolve(const std::vector<Stmt *> statements);

//  private:
    ScopeMap globals;
    std::vector<ScopeMap *> scopes;
    ScopeTracker scope_tracker;
    std::map<const Expr *, size_t> scope_mappings;

    ClassType current_class = NOT_IN_CLASS;
    FunctionType current_function = NOT_IN_FN;

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

    void begin_scope();
    void end_scope();
    void resolve(const Stmt *);
    void resolve(const Expr *);
    void resolve_local(const Expr *expr, const Token name);
    void resolve_fn(FunctionType declaration, const FuncStmt *fn);
    void declare(const Token);
    void define(const Token);

};

#endif //_NL_RESOLVER_H_
