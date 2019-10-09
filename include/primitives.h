#ifndef _NL_PRIMITIVES_H_
#define _NL_PRIMITIVES_H_

#include <memory>
#include <string>

#include "type.h"

using std::shared_ptr;

namespace Primitives
{
  shared_ptr<Type> String() {
    static shared_ptr<Type> type = std::make_shared<Type>("String");
    return type;
  }

  shared_ptr<Type> Int() {
    static shared_ptr<Type> type = std::make_shared<Type>("Int");
    return type;
  }

  shared_ptr<Type> Float() {
    static shared_ptr<Type> type = std::make_shared<Type>("Float");
    return type;
  }

  shared_ptr<Type> Bool() {
    static shared_ptr<Type> type = std::make_shared<Type>("Bool");
    return type;
  }
}

#endif  // _NL_PRIMITIVES_H_
