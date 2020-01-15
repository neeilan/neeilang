#include "type-parse.h"

TypeParse InferredType() {
  TypeParse tp;
  tp.inferred = true;
  return tp;
}
