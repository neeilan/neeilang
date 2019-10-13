#ifndef _NL_TYPE_H_
#define _NL_TYPE_H_

#include <memory>
#include <string>
#include <vector>

#include "field.h"

class Type
{
  public:
    Type(std::string name) : name(name) {}

    bool superclass_of(const Type & other) const {
      const Type * super = & other;
      while (super != nullptr && super != this) {
        super = super->supertype.get();
      }
      return super == this;
    }

    bool subclass_of(const Type & other) const {
      return other.superclass_of(*this);
    }
  
    bool defined = false; // Not just declared, but fully defined.

  private:
    std::string name;
    std::shared_ptr<Type> supertype;
    std::vector<Field> fields;
};

#endif  // _NL_TYPE_H_
