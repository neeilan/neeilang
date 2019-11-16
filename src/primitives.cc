#include "primitives.h"

namespace Primitives {
NLType Class() {
  static NLType type = std::make_shared<Type>("Class");
  return type;
}

NLType String() {
  static NLType type = std::make_shared<Type>("String");
  return type;
}

NLType Int() {
  static NLType type = std::make_shared<Type>("Int");
  return type;
}

NLType Float() {
  static NLType type = std::make_shared<Type>("Float");
  return type;
}

NLType Bool() {
  static NLType type = std::make_shared<Type>("Bool");
  return type;
}

NLType Void() {
  static NLType type = std::make_shared<Type>("Void");
  return type;
}

NLType TypeError() {
  //  Use '$' here as real type names can't contain that character.
  static NLType type = std::make_shared<Type>("$TypeError");
  return type;
}
} // namespace Primitives
