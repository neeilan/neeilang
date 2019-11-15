#ifndef _NL_FUNCTYPE_H_
#define _NL_FUNCTYPE_H_

#include "nltype.h"
#include "stmt.h"

#include <string>
#include <vector>

class Type;

struct FuncType {
  std::string name;
  NLType return_type;
  std::vector<NLType> arg_types;
  bool accepts_args(std::vector<NLType> &supplied_types);
};

#endif // _NL_FUNCTYPE_H_
