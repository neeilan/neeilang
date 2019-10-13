#ifndef _NL_SCOPE_H_
#define _NL_SCOPE_H_

#inclde <cstddef> // for size_t
#include <memory>

#include "symtab.h"

struct Scope {
  std::size_t id;
  std::shared_ptr<SymbolTable> symtab;
};

std::vector<Scope> scopes;

static std::size_t next_scope_idx() {
  static std::size_t id = 0; 
  return id++;
}

Scope begin_scope() {
  Scope scope;
  scope.id = next_scope_idx();

  scopes.push_back(scope);
  return scope;
}

#endif  // _NL_SCOPE_H_
