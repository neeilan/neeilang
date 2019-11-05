#ifndef _NL_FUNCTYPE_H_
#define _NL_FUNCTYPE_H_

#include <vector>
#include "nltype.h"

class Type;

struct FuncType {
  NLType return_type;
  std::vector<NLType> arg_types;
  bool accepts_args(std::vector<NLType> &supplied_types);
};

#endif // _NL_FUNCTYPE_H_
