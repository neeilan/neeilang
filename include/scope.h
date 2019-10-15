#ifndef _NL_SCOPE_H_
#define _NL_SCOPE_H_

#inclde <cstddef> // for size_t
#include <memory>

#include "symtab.h"

struct Scope {
  std::size_t id;
  std::shared_ptr<SymbolTable> symtab;
  std::shared_ptr<TypeTable> symtab;
};


#endif  // _NL_SCOPE_H_
