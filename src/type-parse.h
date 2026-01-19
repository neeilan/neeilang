#ifndef _NL_TYPE_PARSE_H_
#define _NL_TYPE_PARSE_H_

#include <cstdint>
#include <vector>
#include <iostream>

#include "token.h"
#include "name.h"

class Expr;
class SentinelExpr;

struct TypeParse {
  QualifiedName name;
  std::vector<const Expr *> dims;
  bool inferred = false;
  bool isConst = false;
  bool isVariadic = false;
  uint8_t ptrDepth = 0;
  bool isLvalRef = false;
  bool isRvalOrUniversalRef = false;
  Expr* declTypeExpr = nullptr;
  std::string declTypeDesc = "(decltype)"; // Set by AST printer

  TypeParse() = default;
  TypeParse(const TypeParse &) = default;

  // TODO: Make this canonicalName?
  std::string prettyName() const {
    std::string refPart = isLvalRef ? "&" : isRvalOrUniversalRef ? "&&" : "";
    std::string constPart = isConst ? "const " : "";
    std::string namePart = declTypeExpr ? declTypeDesc : name.str();
    return refPart + constPart + namePart + std::string(ptrDepth, '*');
  }

  
  bool is_array() const { return dims.size() > 0; }
  unsigned array_dims() const { return dims.size(); }

  static const Expr *EmptyArrayDim();

private:
  static const SentinelExpr *empty_dim_sentinel;
};

TypeParse InferredType();

#endif // _NL_TYPE_PARSE_H_
