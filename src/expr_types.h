#ifndef _NL_EXPR_TYPES_H_
#define _NL_EXPR_TYPES_H_

#include <memory>

#include "expr.h"
#include "type.h"

using ExprTypes = std::map<const Expr*, std::shared_ptr<Type>>;

#endif  // _NL_EXPR_TYPES_H_
