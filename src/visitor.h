#ifndef _NL_VISITOR_H_
#define _NL_VISITOR_H_

#include <string>

#include "visitable.h"

template <typename T = void> class StmtVisitor {
public:
  virtual ~StmtVisitor() = default;
  virtual T visit(const BlockStmt *) = 0;
  virtual T visit(const ExprStmt *) = 0;
  virtual T visit(const PrintStmt *) = 0;
  virtual T visit(const VarStmt *) = 0;
  virtual T visit(const ClassStmt *) = 0;
  virtual T visit(const IfStmt *) = 0;
  virtual T visit(const WhileStmt *) = 0;
  virtual T visit(const FuncStmt *) = 0;
  virtual T visit(const ReturnStmt *) = 0;
};

template <typename T = void> class ExprVisitor {
public:
  virtual ~ExprVisitor() = default;
  virtual T visit(const Unary *) = 0;
  virtual T visit(const Binary *) = 0;
  virtual T visit(const Grouping *) = 0;
  virtual T visit(const StrLiteral *) = 0;
  virtual T visit(const NumLiteral *) = 0;
  virtual T visit(const BoolLiteral *) = 0;
  virtual T visit(const Variable *) = 0;
  virtual T visit(const Assignment *) = 0;
  virtual T visit(const Logical *) = 0;
  virtual T visit(const Call *) = 0;
  virtual T visit(const Get *) = 0;
  virtual T visit(const Set *) = 0;
  virtual T visit(const GetIndex *) = 0;
  virtual T visit(const SetIndex *) = 0;
  virtual T visit(const This *) = 0;
  virtual T visit(const SentinelExpr *) = 0;
};

#define OVERRIDE_STMT_VISITOR_FNS(T)           \
  virtual T visit(const BlockStmt *) override; \
  virtual T visit(const ExprStmt *) override;  \
  virtual T visit(const PrintStmt *) override; \
  virtual T visit(const VarStmt *) override;   \
  virtual T visit(const ClassStmt *) override; \
  virtual T visit(const IfStmt *) override;    \
  virtual T visit(const WhileStmt *) override; \
  virtual T visit(const FuncStmt *) override;  \
  virtual T visit(const ReturnStmt *) override;

#define OVERRIDE_EXPR_VISITOR_FNS(T)              \
  virtual T visit(const Unary *) override;        \
  virtual T visit(const Binary *) override;       \
  virtual T visit(const Grouping *) override;     \
  virtual T visit(const StrLiteral *) override;   \
  virtual T visit(const NumLiteral *) override;   \
  virtual T visit(const BoolLiteral *) override;  \
  virtual T visit(const Variable *) override;     \
  virtual T visit(const Assignment *) override;   \
  virtual T visit(const Logical *) override;      \
  virtual T visit(const Call *) override;         \
  virtual T visit(const Get *) override;          \
  virtual T visit(const Set *) override;          \
  virtual T visit(const GetIndex *) override;     \
  virtual T visit(const SetIndex *) override;     \
  virtual T visit(const SentinelExpr *) override; \
  virtual T visit(const This *) override;

#endif //_NL_VISITOR_H_
