#include "memory"

#include "primitives.h"
#include "type.h"

using std::shared_ptr;

namespace Primitives {
  shared_ptr<Type> String() {
    static shared_ptr<Type> type = std::make_shared<Type>("String");
    return type;
  }

  shared_ptr<Type> Int() {
    static shared_ptr<Type> type = std::make_shared<Type>("Int");
    return type;
  }

  shared_ptr<Type> Float() {
    static shared_ptr<Type> type = std::make_shared<Type>("Float");
    return type;
  }

  shared_ptr<Type> Bool() {
    static shared_ptr<Type> type = std::make_shared<Type>("Bool");
    return type;
  }

  shared_ptr<Type> Void() {
    static shared_ptr<Type> type = std::make_shared<Type>("Void");
    return type;
  }

  shared_ptr<Type> TypeError() {
    //  Use '$' here as real type names can't contain that character.
    static shared_ptr<Type> type = std::make_shared<Type>("$TypeError");
    return type;
  }
}
