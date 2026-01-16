#ifndef _NL_PARSER_H_
#define _NL_PARSER_H_

#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>

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
  Parser(const std::vector<Token> &tokens) : tokens(tokens) {}
  std::vector<Stmt *> parse();

private:
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
  Expr *unary();
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
  Stmt *enum_declaration();
  Stmt *expression_statement();
  Stmt *if_statement(Token keyword);
  Stmt *while_statement(Token keyword);
  Stmt *for_statement(Token keyword);
  Stmt *return_statement();
  Stmt *func_statement(Specifiers s, std::string kind);

  ParseErr error(Token token, std::string msg);
  void synchronize();
};

#endif //_NL_PARSER_H_
