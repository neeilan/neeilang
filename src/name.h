#ifndef _NL_NAME_H_
#define _NL_NAME_H_

#include <optional>
#include <string>

#include "token.h"

struct TypeParse;

struct QualifiedName {
  std::vector<Token> tokens;
  bool isFullyQualified = false;
  std::vector<std::optional<std::vector<TypeParse*>>> tmplInstantiations;

  QualifiedName& operator+=(QualifiedName const& other) {
    for (size_t i = 0; i < other.tokens.size(); ++i) {
      tokens.push_back(other.tokens[i]);
      tmplInstantiations.push_back(other.tmplInstantiations[i]);
    }
    return *this;
  }

  Token const& token() const {
    return tokens.back();
  }

  std::string str() const;
};

#endif // _NL_NAME_H_
