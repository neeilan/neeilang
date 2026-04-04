#ifndef _NL_EXPR_H_
#define _NL_EXPR_H_

#include "decl-ctx.h"
#include "name.h"
#include "token.h"
#include "type.h"
#include "type-parse.h"
#include "visitor.h"
#include "constval.h"

#include <memory>
#include <string>
#include <variant>
#include <vector>

using std::string;

class Expr {
public:
  virtual void accept(ExprVisitor<void> *visitor) const = 0;
  virtual string accept(ExprVisitor<string> *visitor) const = 0;
  virtual const Expr* accept(ExprVisitor<const Expr*> *visitor) const = 0;

  // TODO: Move into allowedCtxs
  virtual bool lvalue() const { return false; }
  virtual bool is_object_field() const { return false; }
  virtual bool is_indexed() const { return false; }
  virtual bool callable() const { return false; }
  virtual ~Expr(){};

  struct {
    uint8_t typeLike    : 1;
    uint8_t pad_        : 7;
  } allowedCtxs = {};

  DeclCtx::ptr_t ctx;
  std::optional<CompileTimeValue> compileTimeVal;
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

  virtual const Expr* accept(ExprVisitor<const Expr*> *visitor) const {
    return visitor->visit(static_cast<const T *>(this));
  }
};

class Binary : public ExprCRTP<Binary> {
public:
  Binary(const Expr &left, Token op, const Expr &right)
      : left(left), op(op), right(right) {

        if (left.compileTimeVal && right.compileTimeVal) {
          // may be compatible, do + as an example
          if (op.type == PLUS) {
            if (std::holds_alternative<CompileTimeBuiltin>(*left.compileTimeVal)
              && left.compileTimeVal->index() == right.compileTimeVal->index()) {
                auto innerL = std::get<CompileTimeBuiltin>(*left.compileTimeVal);
                auto innerR = std::get<CompileTimeBuiltin>(*right.compileTimeVal);
                if (innerL.index() == innerR.index()) {
                  if (std::holds_alternative<int>(innerL)) {
                    compileTimeVal = CompileTimeValue{std::get<int>(innerL) + std::get<int>(innerR)};
                  } else {
                    compileTimeVal = CompileTimeValue{std::get<double>(innerL) + std::get<double>(innerR)};
                  }
              }
            }
          }
        }

      }

  const Expr &left;
  const Token op;
  const Expr &right;
};

class Grouping : public ExprCRTP<Grouping> {
public:
  explicit Grouping(const Expr &expression) : expression(expression) {}
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
  explicit NumLiteral(std::string value, bool nil = false)
      : value(value), nil(nil) {

        if (value.find('.') != std::string::npos) {
          compileTimeVal = CompileTimeValue{std::stod(value)};
        } else {
          compileTimeVal = CompileTimeValue{std::stoi(value)};
        }
      }

  std::string value;
  bool nil;
  bool has_decimal_point() const {
    return value.find(".") != std::string::npos;
  }
  double as_double() const { return stod(value); }
};

class BoolLiteral : public ExprCRTP<BoolLiteral> {
public:
  explicit BoolLiteral(bool value) : value(value) {}

  bool value;
};

class Unary : public ExprCRTP<Unary> {
public:
  Unary(Token op, const Expr &right) : op(op), right(right) {}

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
  Variable(QualifiedName name) : name(name) {
    allowedCtxs.typeLike = true;
  }

  virtual bool lvalue() const { return true; }
  const QualifiedName name;
};

class Assignment : public ExprCRTP<Assignment> {
public:
  Assignment(QualifiedName name, const Expr &value) : name(name), value(value) {}

  const QualifiedName name;
  const Expr &value;
};

class Logical : public ExprCRTP<Logical> {
public:
  Logical(const Expr &left, Token op, const Expr &right)
      : left(left), op(op), right(right) {}

  const Expr &left;
  const Token op;
  const Expr &right;
};

class Call : public ExprCRTP<Call> {
public:
  Call(const Expr &callee, Token paren, std::vector<Expr *> args)
      : callee(callee), paren(paren), args(args) {}

  const Expr &callee;
  const Token paren;
  const std::vector<Expr *> args;
};

class GetIndex : public ExprCRTP<GetIndex> {
public:
  GetIndex(const Expr &callee, Token bracket, const Expr &index)
      : callee(callee), bracket(bracket), index(index) {}

  virtual bool is_indexed() const { return true; }

  const Expr &callee;
  const Token bracket;
  const Expr &index;
};

class SetIndex : public ExprCRTP<SetIndex> {
public:
  SetIndex(const Expr &callee, Token bracket, const Expr &index,
           const Expr &value)
      : callee(callee), bracket(bracket), index(index), value(value) {}

  const Expr &callee;
  const Token bracket;
  const Expr &index;
  const Expr &value;
};

class Get : public ExprCRTP<Get> {
public:
  Get(const Expr &callee, Token name, TokenType accessOp)
    : callee(callee), name(name), accessOp(accessOp) {}

  virtual bool is_object_field() const { return true; }

  const Expr &callee;
  const Token name;
  TokenType accessOp;
};

class Set : public ExprCRTP<Set> {
public:
  Set(const Expr &callee, Token name, const Expr &value)
      : callee(callee), name(name), value(value) {}

  const Expr &callee;
  const Token name;
  const Expr &value;
};

class SizeOf : public ExprCRTP<SizeOf> {
public:
  explicit SizeOf(std::variant<TypeParse, Expr*> operand)
    : operand(operand) {}
  std::variant<TypeParse, Expr*> operand;
};

class AlignOf : public ExprCRTP<AlignOf> {
public:
  AlignOf(TypeParse typeId)
      : typeId(typeId) {}
  TypeParse typeId;
};

class StaticCast : public ExprCRTP<StaticCast> {
public:
  StaticCast(TypeParse typeId, const Expr* expr)
      : typeId(typeId), expr(expr) {}
  TypeParse typeId;
  const Expr* expr;
};

class SentinelExpr : public ExprCRTP<SentinelExpr> {
  // TODO: Add an ID field for unique'ing.
};

#endif
