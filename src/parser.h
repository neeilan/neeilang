#ifndef _NL_PARSER_H_
#define _NL_PARSER_H_

#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>
#include <unordered_set>

#include "expr.h"
#include "stmt.h"
#include "token.h"
#include "type-parse.h"

class ParseErr : std::runtime_error {
public:
  ParseErr(const std::string &msg) : std::runtime_error(msg) {}
  ParseErr(const char *msg) : std::runtime_error(msg) {}
};

class Parser {
public:
  Parser(const std::vector<Token> &tokens);
  std::vector<Stmt *> parse();

private:
  struct TemplateCtx {
    bool inTemplate = false;
  };
  TemplateCtx templateCtx;

  struct TemplateCtxGuard {
    TemplateCtx& ctx;
    TemplateCtx old;

    explicit TemplateCtxGuard(TemplateCtx& ctx)
      : ctx(ctx), old(ctx) {
      ctx.inTemplate = true;
    }

    ~TemplateCtxGuard() {
      ctx = old;
    }
  };

  struct ClassCtx {
    std::string const* name;
    // Unused, but can be used to determine if directly parsing
    // a member decl by comparing to an overall parse depth.
    int depth = 0;
  };
  std::optional<ClassCtx> classCtx;

  struct ClassCtxGuard {
    std::optional<ClassCtx> & ctx;
    std::optional<ClassCtx> old;

    ClassCtxGuard(
      std::optional<ClassCtx>& ctx,
      std::string const* name
    )
      : ctx(ctx), old(ctx) {
      ctx.emplace();
      ctx->name = name;
    }

    ~ClassCtxGuard() {
      ctx = old;
    }
  };

  bool snoopMode = false;
  struct SnoopGuard {
    int& currRef;
    int startTok;
    bool& snoopModeRef;
    bool oldSnoopMode;

    explicit SnoopGuard(int& curr, bool& snoopMode)
    : currRef(curr), startTok(curr), snoopModeRef(snoopMode), oldSnoopMode(snoopMode) {
      snoopModeRef = true;
    }
    ~SnoopGuard() { currRef = startTok; snoopModeRef = oldSnoopMode; }
  };

  int current = 0; // next token to be used
  std::vector<Token> tokens;
  

  bool match(const std::vector<TokenType> &);
  bool check(const TokenType &type);
  std::optional<Token> consume_any_of(const std::vector<TokenType> &types);
  bool at_end();
  const std::string *outer_class = nullptr;

  QualifiedName consume_qualified_identifier(std::string const& msg);
  Specifiers consume_specifiers();

  Token &advance();
  Token &consume(TokenType type, std::string msg);
  Token &peek();
  Token &peek_ahead();
  Token &previous();
  TypeParse parse_type(const std::string &msg);

  Expr *assignment();
  Expr *bitwise_or();
  Expr *bitwise_and();
  Expr *logical_or();
  Expr *logical_and();
  Expr *expression();
  Expr *equality();
  Expr *comparison();
  Expr *addition();
  Expr *multiplication();
  Expr *modulus();
  Expr *unary();
  Expr *call_like();
  Expr *call();
  Expr *finish_call(Expr *caller);
  Expr *finish_index_get(Expr *expr);
  Expr *primary();

  Stmt *declaration();
  Stmt *var_declaration();
  Stmt *class_declaration();
  Stmt *statement();
  Stmt *print_statement(Token keyword);
  Stmt *block_statement();
  Stmt *namespace_statement();
  Stmt *template_statement();
  Stmt *static_assert_statement();
  Stmt *using_declaration();
  Stmt *enum_declaration();
  Stmt *expression_statement();
  Stmt *if_statement(Token keyword);
  Stmt *while_statement(Token keyword);
  Stmt *for_statement(Token keyword);
  Stmt *return_statement();
  Stmt *func_statement(Specifiers s, std::string kind);

  ParseErr error(Token token, std::string msg);
  void synchronize();
  std::unordered_set<std::string> templateNames;
  bool isTemplateName(const std::string& name);
  std::unordered_set<std::string> typeNames;
  bool isType(const QualifiedName& name);

};

#endif //_NL_PARSER_H_
