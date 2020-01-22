#include "type.h"

struct FuncType;

bool Type::has_field(const std::string &name) {
  if (supertype && supertype->has_field(name)) {
    return true;
  }

  for (auto field : fields) {
    if (field.name == name) {
      return true;
    }
  }

  return false;
}

Field Type::get_field(const std::string &name) {
  if (supertype && supertype->has_field(name)) {
    return supertype->get_field(name);
  }

  assert(has_field(name));

  for (auto field : fields) {
    if (field.name == name) {
      return field;
    }
  }

  // Unreachable
  return Field();
}

int Type::num_fields() {
  int offset = supertype ? supertype->num_fields() : 0;
  return offset + fields.size();
}

int Type::field_idx(const std::string &name) {

  assert(has_field(name));

  if (supertype && supertype->has_field(name)) {
    return supertype->field_idx(name);
  }

  int offset = supertype ? supertype->num_fields() : 0;

  for (int i = 0; i < fields.size(); i++) {
    if (fields[i].name == name) {
      return offset + i;
    }
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

std::vector<std::shared_ptr<FuncType>> Type::get_methods() {
  if (!supertype) {
    return methods;
  }

  std::vector<std::shared_ptr<FuncType>> all_methods;
  auto super_methods = supertype->get_methods();
  for (auto sm : super_methods) {
    all_methods.push_back(sm);
  }

  for (std::shared_ptr<FuncType> m : methods) {
    bool overridden = false;
    for (int i = 0; i < all_methods.size(); i++) {
      if (m->name == all_methods[i]->name) {
        overridden = true;
        all_methods[i] = m;
        break;
      }
    }
    if (!overridden) {
      all_methods.push_back(m);
    }
  }

  return all_methods;
}
