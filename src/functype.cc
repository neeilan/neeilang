#include <memory>
#include <vector>

#include "functype.h"
#include "type.h"

bool FuncType::accepts_args(
    std::vector<std::shared_ptr<Type>> &supplied_types) {
  if (arg_types.size() != supplied_types.size()) {
    return false;
  }

  for (int i = 0; i < arg_types.size(); i++) {
    std::shared_ptr<Type> arg_type = arg_types[i];
    std::shared_ptr<Type> supplied_type = supplied_types[i];
    if (!supplied_type->subclass_of(arg_type.get())) {
      return false;
    }
  }

  return true;
}