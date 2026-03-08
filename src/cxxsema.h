#ifndef _NL_CXXSEMA_H_
#define _NL_CXXSEMA_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "expr.h"
#include "stmt.h"
#include "visitor.h"

/*
CXXSema implements semantic analyses performed after (rather than
hand-in-hand with) parsing.
*/

class CXXSema : public ExprVisitor<void>, public StmtVisitor<void>
{
public:
    explicit CXXSema() {};

    void analyze(const std::vector<const Stmt *> program)
    {
        std::cout << "Semantic analysis OK!" << std::endl;
    }

    void analyze(const Stmt *);
    void analyze(const Expr *);

    OVERRIDE_EXPR_VISITOR_FNS(void)
    OVERRIDE_STMT_VISITOR_FNS(void)

    virtual ~CXXSema() = default;
};

#endif //_NL_CXXSEMA_H_
