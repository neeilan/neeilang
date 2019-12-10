#ifndef _NL_SCOPE_H_
#define _NL_SCOPE_H_

#include <cstddef> // for size_t
#include <memory>

#include "symtab.h"
#include "type-table.h"

struct Scope {
  std::size_t id;
  std::size_t parent = -1;
  std::shared_ptr<SymbolTable> symtab;
  std::shared_ptr<TypeTable> typetab;

  Scope(std::size_t id) : id(id) {
    symtab = std::make_shared<SymbolTable>();
    typetab = std::make_shared<TypeTable>();
  }

  Scope create_child(std::size_t child_id) {
    Scope child;
    child.id = child_id;
    child.parent = id;
    child.symtab = std::make_shared<SymbolTable>(symtab);
    child.typetab = std::make_shared<TypeTable>(typetab);
    return child;
  }

private:
  Scope() {}
};

#endif // _NL_SCOPE_H_
