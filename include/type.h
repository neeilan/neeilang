#ifndef _NL_TYPE_H_
#define _NL_TYPE_H_

#include <memory>
#include <string>

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

  private:
    std::string name;
    std::shared_ptr<Type> supertype;

};

#endif  // _NL_TYPE_H_
