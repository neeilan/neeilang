#ifndef _NL_VISITOR_H_
#define _NL_VISITOR_H_

#include <string>

#include "visitable.h"

template <class T = void> class StmtVisitor {
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

template <class T = void> class ExprVisitor {
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
};

#define STMT_VISITOR_METHODS(T)                                                \
  virtual T visit(const BlockStmt *);                                          \
  virtual T visit(const ExprStmt *);                                           \
  virtual T visit(const PrintStmt *);                                          \
  virtual T visit(const VarStmt *);                                            \
  virtual T visit(const ClassStmt *);                                          \
  virtual T visit(const IfStmt *);                                             \
  virtual T visit(const WhileStmt *);                                          \
  virtual T visit(const FuncStmt *);                                           \
  virtual T visit(const ReturnStmt *);

#define EXPR_VISITOR_METHODS(T)                                                \
  virtual T visit(const Unary *);                                              \
  virtual T visit(const Binary *);                                             \
  virtual T visit(const Grouping *);                                           \
  virtual T visit(const StrLiteral *);                                         \
  virtual T visit(const NumLiteral *);                                         \
  virtual T visit(const BoolLiteral *);                                        \
  virtual T visit(const Variable *);                                           \
  virtual T visit(const Assignment *);                                         \
  virtual T visit(const Logical *);                                            \
  virtual T visit(const Call *);                                               \
  virtual T visit(const Get *);                                                \
  virtual T visit(const Set *);                                                \
  virtual T visit(const GetIndex *);                                           \
  virtual T visit(const SetIndex *);                                           \
  virtual T visit(const This *);

#endif //_NL_VISITOR_H_
