#ifndef _NL_TYPE_TABLE_H_
#define _NL_TYPE_TABLE_H_

#include <memory>
#include <string>

#include "cactus_table.h"
#include "type.h"

using TypeTable = CactusTable<const std::string, std::shared_ptr<Type>>;

#endif  // _NL_TYPE_TABLE_H_
