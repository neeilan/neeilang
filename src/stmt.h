#ifndef _NL_STMT_H_
#define _NL_STMT_H_

#include <string>
#include <vector>
#include <optional>

#include "expr.h"
#include "token.h"
#include "type-parse.h"
#include "visitor.h"

using std::string;

struct TemplateArg {
  Token name;
  bool isVariadic = false;
};

struct NamedEnumerator {
  Token name;
  std::optional<Token> value;
};

struct Specifiers {
  union {
  struct {
  uint16_t isConst     : 1;
  uint16_t isConstexpr : 1;
  uint16_t isConsteval : 1;
  uint16_t isStatic    : 1;
  uint16_t isInline    : 1;
  uint16_t isVirtual   : 1;
  uint16_t isExplicit  : 1;
  uint16_t isFriend    : 1;
  uint16_t isExtern    : 1;
  uint16_t isNoexcept  : 1;
  uint16_t _pad        : 6;
  } f;
  uint16_t bits;
  };

  bool any() const {
    return bits != 0;
  }

  std::string str() const {
    std::string res;

    auto add = [&res](const char* s) {
      if (!res.empty())
        res += ' ';

      // lowercase first char after "is"
      res += static_cast<char>(
        std::tolower(static_cast<unsigned char>(s[2]))
      );
      // append the rest
      res += (s + 3);
    };

#define SP_HANDLE(x) \
    if (f.x) add(#x);

    SP_HANDLE(isConst)
    SP_HANDLE(isConstexpr)
    SP_HANDLE(isConsteval)
    SP_HANDLE(isStatic)
    SP_HANDLE(isInline)
    SP_HANDLE(isVirtual)
    SP_HANDLE(isExplicit)
    SP_HANDLE(isFriend)
    SP_HANDLE(isExtern)
    SP_HANDLE(isNoexcept)

#undef SP_HANDLE

    return res;
  }
};

struct Attributes {
  uint8_t isPacked    : 1;
  uint8_t isNoDiscard : 1;
};

class Stmt {
public:
  virtual void accept(StmtVisitor<void> *visitor) const = 0;
  virtual string accept(StmtVisitor<string> *visitor) const = 0;
  virtual ~Stmt() = default;

  struct {
    uint8_t classMember : 1;
    uint8_t classVar    : 1;
    uint8_t fnLike      : 1;
    uint8_t pad_        : 6;
  } allowedCtxs = {};
};

template <typename T> class StmtCRTP : public Stmt {
public:
  virtual void accept(StmtVisitor<void> *visitor) const {
    return visitor->visit(static_cast<const T *>(this));
  }

  virtual string accept(StmtVisitor<string> *visitor) const {
    return visitor->visit(static_cast<const T *>(this));
  }
};

class ExprStmt : public StmtCRTP<ExprStmt> {
public:
  explicit ExprStmt(const Expr *expression, const Token &sc)
      : expression(expression), sc(sc) {}

  const Expr *expression = nullptr;
  const Token &sc;
};

class PrintStmt : public StmtCRTP<PrintStmt> {
public:
  explicit PrintStmt(const Token keyword, const Expr *expression)
      : keyword(keyword), expression(expression) {}

  const Token keyword;
  const Expr *expression = nullptr;
};

class VarStmt : public StmtCRTP<VarStmt> {
public:
  explicit VarStmt(const Token name, const TypeParse tp,
                   const Expr *initializer)
      : name(name), tp(tp), expression(initializer) {
        allowedCtxs.classMember = true;
        allowedCtxs.classVar = true;
      }

  const Token name;
  const TypeParse tp;
  const Expr *expression = nullptr;
};

class BlockStmt : public StmtCRTP<BlockStmt> {
public:
  explicit BlockStmt(std::vector<Stmt *> block_contents)
      : block_contents(block_contents) {}

  std::vector<Stmt *> block_contents;
};

class NamespaceStmt : public StmtCRTP<NamespaceStmt> {
public:
  explicit NamespaceStmt(std::string name, std::vector<Stmt *> contents)
      : name(std::move(name)), contents(std::move(contents)) {}

  std::string name;
  std::vector<Stmt *> contents;
};

class TemplateStmt :  public StmtCRTP<TemplateStmt> {
public:
  explicit TemplateStmt(std::vector<TemplateArg> args, Stmt * fnOrClass)
    : args(args), fnOrClass(fnOrClass) {
        allowedCtxs.classMember = true;
    }

  std::vector<TemplateArg> args;
  Stmt * fnOrClass;
};

class StaticAssertStmt :  public StmtCRTP<StaticAssertStmt> {
public:
  explicit StaticAssertStmt(const Expr * value) : value(value) {
      allowedCtxs.classMember = true;
  };
  const Expr * value;
};

class ScopedEnum :  public StmtCRTP<ScopedEnum> {
public:
  explicit ScopedEnum(std::string name, std::vector<NamedEnumerator> enumerators, std::optional<TypeParse> underlying)
    : name(name), enumerators(enumerators), underlying(underlying) {
      allowedCtxs.classMember = true;
    }

  std::string name;
  std::vector<NamedEnumerator> enumerators;
  std::optional<TypeParse> underlying;
};


class IfStmt : public StmtCRTP<IfStmt> {
public:
  explicit IfStmt(const Token keyword, Expr *condition, Stmt *then_branch,
                  Stmt *else_branch)
      : keyword(keyword), condition(condition), then_branch(then_branch),
        else_branch(else_branch) {}

  const Token keyword;
  const Expr *condition;
  const Stmt *then_branch;
  const Stmt *else_branch;
};

class WhileStmt : public StmtCRTP<WhileStmt> {
public:
  explicit WhileStmt(Token while_tok, Expr *condition, Stmt *body)
      : while_tok(while_tok), condition(condition), body(body) {}

  const Token while_tok;
  const Expr *condition;
  const Stmt *body;
};

class FuncStmt : public StmtCRTP<FuncStmt> {
public:
  explicit FuncStmt(Token name, std::vector<Token> parameters,
                    std::vector<TypeParse> parameter_types,
                    TypeParse return_type, std::vector<Stmt *> body)
      : name(name), parameters(parameters), parameter_types(parameter_types),
        return_type(return_type), body(body) {
      allowedCtxs.classMember = true;
      allowedCtxs.fnLike = true;
    }

  bool is_void() const { return return_type.prettyName() == "void"; }

  const Token name;
  const std::vector<Token> parameters;
  const std::vector<TypeParse> parameter_types;
  const TypeParse return_type;
  const std::vector<Stmt *> body;
  bool isStatic = false;
  Specifiers specifiers;
  std::optional<TokenType> operatorOverload;

  void setSpecifiers(Specifiers s) {
    specifiers = s;
  }

  void setOperatorOverload(std::optional<TokenType> o) {
    operatorOverload = std::move(o);
  }
};

class ReturnStmt : public StmtCRTP<ReturnStmt> {
public:
  explicit ReturnStmt(Token keyword, Expr *value)
      : keyword(keyword), value(value) {}

  const Token keyword;
  const Expr *value;
};

class ClassStmt : public StmtCRTP<ClassStmt> {
public:
  explicit ClassStmt(Token name, Token *superclass, std::vector<Token> fields,
                     std::vector<TypeParse> field_types,
                     std::vector<Stmt *> memberDecls)
      : name(name), superclass(superclass), fields(fields),
        field_types(field_types), memberDecls(memberDecls) {
      allowedCtxs.classMember = true;
    }

  const Token name;
  const Token *superclass = nullptr;
  const std::vector<Token> fields;
  const std::vector<TypeParse> field_types;
  const std::vector<Stmt *> memberDecls;

  const std::vector<Stmt *> methods() const {
    std::vector<Stmt *> m;
    for (auto * s : memberDecls) {
      if (s->allowedCtxs.fnLike) {
        m.push_back(s);
      }
    }
    return m;
  }
};

#endif // _NL_STMT_H_
