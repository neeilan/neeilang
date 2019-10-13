#ifndef _NL_PRIMITIVES_H_
#define _NL_PRIMITIVES_H_

#include <memory>

#include "type.h"

using std::shared_ptr;

namespace Primitives
{
  shared_ptr<Type> String();
  shared_ptr<Type> Int();
  shared_ptr<Type> Float();
  shared_ptr<Type> Bool();
  shared_ptr<Type> Void();
  shared_ptr<Type> TypeError();
}

#endif  // _NL_PRIMITIVES_H_
