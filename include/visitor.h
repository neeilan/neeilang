#ifndef _NL_VISITOR_H_
#define _NL_VISITOR_H_

#include <string>

#include "visitable.h"
#include "expr.h"


template <class T>
class StmtVisitor {
public:
    virtual ~StmtVisitor() = default;
    virtual T visit(const BlockStmt*) = 0;
    virtual T visit(const ExprStmt*) = 0;
    virtual T visit(const PrintStmt*) = 0;
    virtual T visit(const VarStmt*) = 0;
    virtual T visit(const ClassStmt*) = 0;
    virtual T visit(const IfStmt*) = 0;
    virtual T visit(const WhileStmt*) = 0;
    virtual T visit(const FuncStmt*) = 0;
    virtual T visit(const ReturnStmt*) = 0;
};

template <class T>
class ExprVisitor {
public:
    virtual ~ExprVisitor() = default;
    virtual T visit(const Unary*) = 0;
    virtual T visit(const Binary*) = 0;
    virtual T visit(const Grouping*) = 0;
    virtual T visit(const StrLiteral*) = 0;
    virtual T visit(const NumLiteral*) = 0;
    virtual T visit(const BoolLiteral*) = 0;
    virtual T visit(const Variable*) = 0;
    virtual T visit(const Assignment*) = 0;
    virtual T visit(const Logical*) = 0;
    virtual T visit(const Call*) = 0;
    virtual T visit(const Get*) = 0;
    virtual T visit(const Set*) = 0;
    virtual T visit(const This*) = 0;
};

#endif //_NL_VISITOR_H_
