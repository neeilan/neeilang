#ifndef _NL_SCANNER_H_
#define _NL_SCANNER_H_

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>
#include <stack>

#include "token.h"

class Scanner {
public:
  Scanner(const std::string &path);

  std::vector<Token> scan_tokens();

private:
  struct SourceCtx {
    const std::string source;
    const char* path;
    std::vector<const char*> inclPath;
    size_t start = 0;
    size_t current = 0;
    int line = 1;
  };

  void add_ctx(const std::filesystem::path& path, std::optional<size_t> inclLine = std::nullopt);
  SourceCtx& ctx();
  std::vector<Token> tokens;
  std::stack<SourceCtx> ctxs;
  std::vector<std::string> fnames; // unordered strtab for filenames

  static const std::map<std::string, TokenType> keywords;

  void preprocessor();

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