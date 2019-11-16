#include <memory>

#include "nltype.h"
#include "type.h"

namespace NLTypeUtil {
NLType create(const std::string name) { return std::make_shared<Type>(name); }
} // namespace NLTypeUtil
