#ifndef _NL_FIELD_H_
#define _NL_FIELD_H_

#include <memory>
#include <string>

#include "nltype.h"

class Type;

struct Field {
  std::string name;
  NLType type;
};

#endif // _NL_FIELD_H_
