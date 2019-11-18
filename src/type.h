#ifndef _NL_TYPE_H_
#define _NL_TYPE_H_

#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include "field.h"
#include "functype.h"
#include "nltype.h"

struct FuncType;

class Type {
public:
  Type(std::string name) : name(name) {}

  bool superclass_of(const Type *other) const {
    const Type *super = other;
    while (super != nullptr && super != this) {
      super = super->supertype.get();
    }
    return super == this;
  }

  bool subclass_of(const Type *other) const {
    return other->superclass_of(this);
  }

  bool has_field(const std::string &name);
  Field get_field(const std::string &name);
  int field_idx(const std::string &name);
  bool has_method(const std::string &name);
  std::shared_ptr<FuncType> get_method(const std::string &name);

  std::string name;
  std::shared_ptr<Type> supertype;
  std::vector<Field> fields;
  std::vector<std::shared_ptr<FuncType>> methods;

  std::shared_ptr<FuncType> functype = nullptr;
};

#endif // _NL_TYPE_H_
