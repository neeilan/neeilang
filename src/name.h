#ifndef _NL_NAME_H_
#define _NL_NAME_H_

#include <optional>
#include <string>

#include "token.h"

struct TypeParse;

struct QualifiedName {
  std::vector<Token> tokens;
  bool isFullyQualified = false;
  std::optional<std::vector<TypeParse*>> tmplInstantiation;

  Token const& token() const {
    return tokens.back();
  }

  std::string str() const;
};

#endif // _NL_NAME_H_
