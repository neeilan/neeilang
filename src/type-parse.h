#ifndef _NL_TYPE_PARSE_H_
#define _NL_TYPE_PARSE_H_

#include <vector>

#include "expr.h"
#include "token.h"

struct TypeParse {
  Token name;
  bool arr;
  std::vector<const Expr *> dims;
};

#endif // _NL_TYPE_PARSE_H_
