#ifndef _NL_NAME_H_
#define _NL_NAME_H_

#include <string>

#include "token.h"

struct QualifiedName {
  std::vector<Token> tokens;
  bool isFullyQualified = false;

  Token const& token() const {
    return tokens.back();
  }

  std::string str() const {
    std::string res = isFullyQualified ? "::" : "";
    for (auto const&s : tokens) {
      res += s.lexeme;
      if (&s != &tokens.back()) {
        res += "::";
      }
    }
    return res;
  }
};

#endif // _NL_NAME_H_
