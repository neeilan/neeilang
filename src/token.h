#ifndef _NL_TOKEN_H_
#define _NL_TOKEN_H_

#include <string>
#include <vector>

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
  STATIC,
  INLINE,
  CONST,
  CONSTEVAL,
  CONSTEXPR,
  EXPLICIT,
  VIRTUAL,
  FRIEND,
  EXTERN,
  NOEXCEPT,
  RESERVED_KEYWORD
};

class Token {
public:
  TokenType type;
  std::string lexeme;
  std::string literal;
  int line;
  // [included-from 1, ..., included-from n, file-token-is-in]
  std::vector<const char*> inclPath;

  Token() {}

  Token(
    TokenType type,
    const std::string &lexeme,
    const std::string &literal,
    int line,
    std::vector<const char *> const& inclPath)
      : type(type)
      , lexeme(lexeme)
      , literal(literal)
      , line(line)
      , inclPath(inclPath) {};

  Token(const Token &token) = default;

  std::string str() const;
};

#endif // _NL_TOKEN_H_
