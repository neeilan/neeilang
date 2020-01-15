#include <map>
#include <sstream>

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

NLType Array(NLType elem_type, int dims) {
  static std::map <NLType, std::map<int, NLType>> array_types;
  auto t =  array_types[elem_type][dims];
  if (!t) {
    std::stringstream name;
    name << elem_type->name;
    for (int i = 0; i < dims; i++) { name << "[]" ; }   
    t = std::make_shared<Type>(name.str());
    t->dims = dims;
    t->underlying_type = elem_type;
    array_types[elem_type][dims] = t;
  }
  return t;
}

NLType TypeError() {
  //  Use '$' here as real type names can't contain that character.
  static NLType type = std::make_shared<Type>("$TypeError");
  return type;
}
} // namespace Primitives
