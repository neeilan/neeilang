#ifndef _NL_FUNCTYPE_H_
#define _NL_FUNCTYPE_H_

#include <cassert>
#include <memory>
#include <vector>

class Type;

struct FuncType
{
    std::shared_ptr<Type> return_type;
    std::vector<std::shared_ptr<Type>> arg_types;
    bool accepts_args(std::vector<std::shared_ptr<Type>> & supplied_types);
};

#endif  // _NL_FUNCTYPE_H_
