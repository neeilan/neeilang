#ifndef _NL_SCANNER_H_
#define _NL_SCANNER_H_

#include <map>
#include <string>
#include <vector>

#include "token.h"

class Scanner {
public:
  Scanner(const std::string &source);

  std::vector<Token> scan_tokens();

private:
  const std::string source;
  std::vector<Token> tokens;
  static const std::map<std::string, TokenType> keywords;

  int start = 0;
  int current = 0;
  int line = 1;

  bool is_at_end();

  bool is_alpha(char c);

  bool is_alphanumeric(char c);

  bool is_digit(char c);

  bool match(char expected);

  char advance();

  char peek();

  char peek_next();

  void add_token(TokenType type);

  void add_token(TokenType type, std::string literal);

  void scan_token();

  void string();

  void number();

  void identifier();
};

#endif // _NL_SCANNER_H_