#ifndef _NL_TYPE_H_
#define _NL_TYPE_H_

#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include "nltype.h"
#include "field.h"
#include "functype.h"

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

  bool has_field(std::string name) {
    for (auto field : fields) {
      if (field.name == name)
        return true;
    }
    return false;
  }

  Field get_field(std::string name) {
    assert(has_field(name));
    for (auto field : fields) {
      if (field.name == name)
        return field;
    }
    return Field{"NO_SUCH_FIELD", nullptr}; // Unreachable
  }

  std::string
      name; // TODO : look into using Token here to preserve source info.
  std::shared_ptr<Type> supertype;
  std::vector<Field> fields;
  std::shared_ptr<FuncType> functype = nullptr;
};

#endif // _NL_TYPE_H_
