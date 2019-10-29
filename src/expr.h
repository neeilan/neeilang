#ifndef _NL_EXPR_H_
#define _NL_EXPR_H_

#include "token.h"
#include "type.h"
#include "visitor.h"

#include <memory>
#include <string>
#include <vector>

using std::string;

class Expr {
public:
  virtual void accept(ExprVisitor<void> *visitor) const = 0;
  virtual string accept(ExprVisitor<string> *visitor) const = 0;

  virtual bool lvalue() const { return false; }
  virtual bool is_object_field() const { return false; }
  virtual bool callable() const { return false; }
  virtual ~Expr(){};
};

// Use CRTP (https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern)
// to avoid
// having each derived Expr type needing to be aware of potential visitors. We
// avoid having to
// repeat the 'accept' methods in each subtype.
template <typename T> class ExprCRTP : public Expr {
  virtual void accept(ExprVisitor<void> *visitor) const {
    return visitor->visit(static_cast<const T *>(this));
  }

  virtual string accept(ExprVisitor<string> *visitor) const {
    return visitor->visit(static_cast<const T *>(this));
  }
};

class Binary : public ExprCRTP<Binary> {
public:
  Binary(Expr &left, Token op, Expr &right)
      : left(left), op(op), right(right) {}

  const Expr &left;
  const Token op;
  const Expr &right;
};

class Grouping : public ExprCRTP<Grouping> {
public:
  explicit Grouping(Expr &expression) : expression(expression) {}
  const Expr &expression;
};

class StrLiteral : public ExprCRTP<StrLiteral> {
public:
  explicit StrLiteral(const std::string &value, bool nil = false)
      : value(value), nil(nil) {}

  std::string value;
  bool nil;
};

class NumLiteral : public ExprCRTP<NumLiteral> {
public:
  explicit NumLiteral(double value, bool nil = false)
      : value(value), nil(nil) {}

  double value;
  bool nil;
};

class BoolLiteral : public ExprCRTP<BoolLiteral> {
public:
  explicit BoolLiteral(bool value) : value(value) {}

  bool value;
};

class Unary : public ExprCRTP<Unary> {
public:
  Unary(Token op, Expr &right) : op(op), right(right) {}

  const Token op;
  const Expr &right;
};

class This : public ExprCRTP<This> {
public:
  This(Token keyword) : keyword(keyword) {}

  const Token keyword;
};

class Variable : public ExprCRTP<Variable> {
public:
  Variable(Token name) : name(name) {}

  virtual bool lvalue() const { return true; }
  const Token name;
};

class Assignment : public ExprCRTP<Assignment> {
public:
  Assignment(Token name, Expr &value) : name(name), value(value) {}

  const Token name;
  const Expr &value;
};

class Logical : public ExprCRTP<Logical> {
public:
  Logical(Expr &left, Token op, Expr &right)
      : left(left), op(op), right(right) {}

  const Expr &left;
  const Token op;
  const Expr &right;
};

class Call : public ExprCRTP<Call> {
public:
  Call(Expr &callee, Token paren, std::vector<Expr *> args)
      : callee(callee), paren(paren), args(args) {}

  const Expr &callee;
  const Token paren;
  const std::vector<Expr *> args;
};

class Get : public ExprCRTP<Get> {
public:
  Get(Expr &callee, Token name) : callee(callee), name(name) {}

  virtual bool is_object_field() const { return true; }

  Expr &callee;
  const Token name;
};

class Set : public ExprCRTP<Set> {
public:
  Set(Expr &callee, Token name, Expr &value)
      : callee(callee), name(name), value(value) {}

  Expr &callee;
  const Token name;
  const Expr &value;
};

#endif