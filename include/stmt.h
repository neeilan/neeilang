#ifndef _NL_STMT_H_
#define _NL_STMT_H_

#include <string>
#include <vector>

#include "token.h"
#include "expr.h"
#include "visitor.h"

using std::string;

class Stmt
{
  public:
    virtual void accept(StmtVisitor<void> * visitor) const = 0;
    virtual string accept(StmtVisitor<string> * visitor) const = 0;
    virtual ~Stmt() = default;
};

template <typename T>
class StmtCRTP : public Stmt {
  public:
    virtual void accept(StmtVisitor<void> * visitor) const
    {
        return visitor->visit(static_cast<const T *>(this));
    }

    virtual string accept(StmtVisitor<string> * visitor) const
    {
        return visitor->visit(static_cast<const T *>(this));
    }
};

class ExprStmt : public StmtCRTP<ExprStmt>
{
  public:
    explicit ExprStmt(const Expr * expression)
        : expression(expression) {}

    const Expr *expression = nullptr;
};

class PrintStmt : public StmtCRTP<PrintStmt>
{
  public:
    explicit PrintStmt(const Expr * expression)
        : expression(expression) {}

    const Expr *expression = nullptr;
};

class VarStmt : public StmtCRTP<VarStmt>
{
  public:
    explicit VarStmt(const Token name, const Token type, const Expr * initializer)
        : name(name),
          type(type),
          expression(initializer) {}

    const Token name;
    const Token type;
    const Expr * expression = nullptr;
};

class BlockStmt : public StmtCRTP<BlockStmt>
{
  public:
    explicit BlockStmt(std::vector<Stmt *> block_contents)
        : block_contents(block_contents) {}

    std::vector<Stmt *> block_contents;
};

class IfStmt : public StmtCRTP<IfStmt>
{
  public:
    explicit IfStmt(Expr *condition, Stmt *then_branch, Stmt *else_branch)
        : condition(condition),
          then_branch(then_branch),
          else_branch(else_branch) {}

    const Expr * condition;
    const Stmt * then_branch;
    const Stmt * else_branch;
};

class WhileStmt : public StmtCRTP<WhileStmt>
{
  public:
    explicit WhileStmt(Token while_tok, Expr * condition, Stmt * body)
        : while_tok(while_tok),
          condition(condition),
          body(body) {}

    const Token while_tok;
    const Expr * condition;
    const Stmt * body;
};

class FuncStmt : public StmtCRTP<FuncStmt>
{
  public:
    explicit FuncStmt(
      Token name,
      std::vector<Token> parameters,
      std::vector<Token> parameter_types,
      Token return_type,
      std::vector<Stmt *> body)
        : name(name),
          parameters(parameters),
          parameter_types(parameter_types),
          return_type(return_type),
          body(body) {}

    const Token name;
    const std::vector<Token> parameters;
    const std::vector<Token> parameter_types;
    const Token return_type;
    const std::vector<Stmt *> body;
};

class ReturnStmt : public StmtCRTP<ReturnStmt>
{
  public:
    explicit ReturnStmt(Token keyword, Expr * value)
        : keyword(keyword),
          value(value) {}

    const Token keyword;
    const Expr * value;
};

class ClassStmt : public StmtCRTP<ClassStmt>
{
  public:
    explicit ClassStmt(
      Token name,
      Token * superclass,
      std::vector<Token> fields,
      std::vector<Token> field_types,
      std::vector<Stmt *> methods)
        : name(name),
          superclass(superclass),
          fields(fields),
          field_types(field_types),
          methods(methods) {}

    const Token name;
    const Token * superclass = nullptr;
    const std::vector<Token> fields;
    const std::vector<Token> field_types;
    const std::vector<Stmt *> methods;
};

#endif  // _NL_STMT_H_
