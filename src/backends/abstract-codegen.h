#ifndef _NL_BACKENDS_ABSTRACT_CODEGEN_H_
#define _NL_BACKENDS_ABSTRACT_CODEGEN_H_

#include <vector>

#include "stmt.h"

class AbstractCodegen {
  virtual void generate(const std::vector<Stmt *> &program) = 0;
};

#endif // _NL_BACKENDS_ABSTRACT_CODEGEN_H_
