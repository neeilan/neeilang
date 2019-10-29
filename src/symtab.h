#ifndef _NL_SYMTAB_H_
#define _NL_SYMTAB_H_

#include <memory>
#include <string>

#include "cactus_table.h"
#include "type.h"

struct Symbol {
  std::string name;
  std::shared_ptr<Type> type;
};

using SymbolTable = CactusTable<const std::string, Symbol>;

#endif  // _NL_SYMTAB_H_


