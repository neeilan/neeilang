#ifndef _NL_PARSER_H_
#define _NL_PARSER_H_

#include <memory>
#include <stdexcept>
#include <vector>

#include "token.h"
#include "expr.h"
#include "stmt.h"

class ParseErr : std::runtime_error
{
  public:
    ParseErr(const std::string &msg) : std::runtime_error(msg) {}
    ParseErr(const char *msg) : std::runtime_error(msg) {}
};

class Parser
{
  public:
    Parser(const std::vector<Token> & tokens) : tokens(tokens) {}
    std::vector<Stmt *> parse();

  private:
    int current = 0; // next token to be used
    std::vector<Token> tokens;

    bool match(const std::vector<TokenType> &);
    bool check(const TokenType &type);
    bool at_end();

    Token &advance();
    Token &consume(TokenType type, std::string msg);
    Token &peek();
    Token &peek_ahead();
    Token &previous();

    Expr *assignment();
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
    Expr *primary();

    Stmt *declaration();
    Stmt *var_declaration();
    Stmt *class_declaration();
    Stmt *statement();
    Stmt *print_statement();
    Stmt *block_statement();
    Stmt *expression_statement();
    Stmt *if_statement();
    Stmt *while_statement();
    Stmt *for_statement();
    Stmt *return_statement();
    Stmt *func_statement(std::string kind);

    ParseErr error(Token token, std::string msg);
    void synchronize();
};

#endif //_NL_PARSER_H_