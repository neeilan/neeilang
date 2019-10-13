#ifndef _NL_FIELD_H_
#define _NL_FIELD_H_

#include <memory>
#include <string>

#include "type.h"

class Type;

struct Field
{
  std::string name;
  std::shared_ptr<Type> type;
};

#endif  // _NL_FIELD_H_
