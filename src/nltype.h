#ifndef _NL_NLTYPE_H_
#define _NL_NLTYPE_H_

#include <memory>
#include <string>

class Type;

using NLType = std::shared_ptr<Type>;

namespace NLTypeUtil {
NLType create(const std::string name);
}

#endif // _NL_NLTYPE_H_
