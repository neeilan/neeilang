#ifndef _NL_TYPE_TABLE_H_
#define _NL_TYPE_TABLE_H_

#include <memory>
#include <string>

#include "cactus_table.h"
#include "type.h"
#include "stmt.h"

using TypeTable = CactusTable<const std::string, NLType>;

namespace TypeTableUtil {
  std::string fn_key(const FuncStmt * func);
  std::string fn_key(const std::string func);
}

#endif  // _NL_TYPE_TABLE_H_
