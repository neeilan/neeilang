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
  for (auto field : fields) {
    if (field.name == name)
      return field;
  }
  return Field{"NO_SUCH_FIELD", nullptr}; // Unreachable
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
