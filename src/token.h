#ifndef _NL_TOKEN_H_
#define _NL_TOKEN_H_

#include <string>

enum TokenType {
  // Single-character tokens.
  LEFT_PAREN,
  RIGHT_PAREN,
  LEFT_BRACE,
  RIGHT_BRACE,
  LEFT_BRACKET,
  RIGHT_BRACKET,
  COMMA,
  DOT,
  MINUS,
  PLUS,
  SEMICOLON,
  COLON,
  SLASH,
  STAR,

  // One or two character tokens.
  BANG,
  BANG_EQUAL,
  EQUAL,
  EQUAL_EQUAL,
  GREATER,
  GREATER_EQUAL,
  LESS,
  LESS_EQUAL,
  COLON_COLON,

  // Literals.
  IDENTIFIER,
  STRING,
  NUMBER,

  // Keywords.
  AND,
  CLASS,
  ELSE,
  FALSE,
  FN,
  LAMBDA,
  FOR,
  IF,
  NIL,
  OR,
  PRINT,
  RETURN,
  SUPER,
  THIS,
  TRUE,
  VAR,
  WHILE,
  END_OF_FILE,
  NAMESPACE,
  TEMPLATE,
  TYPENAME,
  ENUM,
  RESERVED_KEYWORD
};

class Token {
public:
  TokenType type;
  std::string lexeme;
  std::string literal;
  int line;
  const char* fname;

  Token() {}

  Token(
    TokenType type,
    const std::string &lexeme,
    const std::string &literal,
    int line,
    const char * fname)
      : type(type)
      , lexeme(lexeme)
      , literal(literal)
      , line(line)
      , fname(fname) {};

  Token(const Token &token) = default;

  std::string str() const;
};

#endif // _NL_TOKEN_H_
