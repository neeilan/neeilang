#include "type_table.h"

namespace TypeTableUtil {
  std::string fn_key(const FuncStmt * func) {
    // TODO : should account for methods by encoding class name in key.
    return fn_key(func->name.lexeme);
  }

  std::string fn_key(const std::string func) {
    return "$fn_" + func;
  }
}
