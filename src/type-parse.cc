#include "type-parse.h"

#include "expr.h"

TypeParse InferredType() {
  TypeParse tp;
  tp.inferred = true;
  return tp;
}

const SentinelExpr * TypeParse::empty_dim_sentinel;
const Expr * TypeParse::EmptyArrayDim() {
  if (!TypeParse::empty_dim_sentinel) {
    TypeParse::empty_dim_sentinel = new SentinelExpr();
  }
  return empty_dim_sentinel;
}
