#ifndef _NL_TYPE_PARSE_H_
#define _NL_TYPE_PARSE_H_

#include <vector>

#include "token.h"

class Expr;

struct TypeParse {
  Token name;
  bool arr;
  std::vector<const Expr *> dims;
  bool inferred = false;
};

TypeParse InferredType();

#endif // _NL_TYPE_PARSE_H_
