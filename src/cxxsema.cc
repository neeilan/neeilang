#include "cxxsema.h"
#include <stdexcept>

void CXXSema::analyze(const Stmt *stmt) { stmt->accept(this); }

void CXXSema::analyze(const Expr *expr)
{
    expr->accept(this);
}

void CXXSema::visit(const Unary *expr)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const Binary *expr)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const NumLiteral *expr)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const Grouping *expr)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const StrLiteral *expr)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const BoolLiteral *expr)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const Logical *expr)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const VarStmt *stmt)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const Variable *expr)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const Assignment *expr)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const Call *expr)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const Get *expr)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const Set *expr)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const This *expr)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const ExprStmt *stmt)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const BlockStmt *stmt)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const IfStmt *stmt)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const PrintStmt *stmt)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const NamespaceStmt *stmt)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const ScopedEnum *)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const TemplateStmt *)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const StaticAssertStmt *)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const UsingStmt *)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const AliasStmt *)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const ClassStmt *stmt)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const WhileStmt *stmt)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const FuncStmt *stmt)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const ReturnStmt *stmt)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const GetIndex *expr)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const SetIndex *expr)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const SizeOf *)
{
    assert(false && "Unimplemented");
}
void CXXSema::visit(const AlignOf *)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const StaticCast *)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const SentinelExpr *expr)
{
    assert(false && "Unimplemented");
}

void CXXSema::visit(const ExplicitClassTemplateInitialization *expr)
{
    assert(false && "Unimplemented");
}