#include <cstdlib>

#include "arrays.h"
#include "primitives.h"

namespace Arrays {
NLType next_enclosed_type(NLType t) {
  assert(t->dims > 0 && "t is not a valid array Type");
  return t->dims == 1 ? t->underlying_type
                      : Primitives::Array(t->underlying_type, t->dims - 1);
}
} // namespace Arrays
