#include <memory>
#include <vector>

#include "functype.h"
#include "type.h"

bool FuncType::accepts_args(
    std::vector<NLType> &supplied_types) {
  if (arg_types.size() != supplied_types.size()) {
    return false;
  }

  for (int i = 0; i < arg_types.size(); i++) {
    NLType arg_type = arg_types[i];
    NLType supplied_type = supplied_types[i];
    if (!supplied_type->subclass_of(arg_type.get())) {
      return false;
    }
  }

  return true;
}
