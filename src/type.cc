#include "type.h"

struct FuncType;

bool Type::has_field(const std::string &name) {
  for (auto field : fields) {
    if (field.name == name)
      return true;
  }
  return false;
}

Field Type::get_field(const std::string &name) {
  assert(has_field(name));
  return fields[field_idx(name)];
}

int Type::field_idx(const std::string &name) {
  assert(has_field(name));
  for (int i = 0; i < fields.size(); i++) {
    if (fields[i].name == name)
      return i;
  }
  return -1; // Unreachable
}

bool Type::has_method(const std::string &name) {
  for (auto method : methods) {
    if (method->name == name)
      return true;
  }
  return false;
}

std::shared_ptr<FuncType> Type::get_method(const std::string &name) {
  assert(has_method(name));
  for (auto method : methods) {
    if (method->name == name)
      return method;
  }
  return std::make_shared<FuncType>(); // Unreachable
}
