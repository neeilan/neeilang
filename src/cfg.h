#ifndef _NL_CFG_H_
#define _NL_CFG_H_

#include <map>
#include <memory>
#include <vector>

#include "stmt.h"
#include "token.h"

namespace NL {

class BasicBlock;

class BasicBlock {
private:
  bool _fn_entry = false;
  bool _is_void = false;
  bool _returns = false;

  std::vector<std::shared_ptr<BasicBlock>> _successors;

public:
  static std::shared_ptr<BasicBlock> fn_entry(bool is_void) {
    return std::make_shared<BasicBlock>(true, is_void);
  }

  static std::shared_ptr<BasicBlock> non_entry() {
    return std::make_shared<BasicBlock>(false, false);
  }

  static std::shared_ptr<BasicBlock> unconditional_return() {
    auto bb = non_entry();
    bb->_returns = true;
    return bb;
  }

  explicit BasicBlock(bool entry, bool is_void)
      : _fn_entry(entry), _is_void(is_void) {}

  void add_successor(std::shared_ptr<BasicBlock> suc);

  /* Returns true iff this BasicBlock ends in an explicit return. */
  bool returns_at_node();

  /* Return true if this is the entry block of a void function */
  bool is_void_func();

  /* Returns true iff all paths through this BasicBlock terminate in an
   * explicit return. */
  bool returns();

  std::vector<std::shared_ptr<NL::BasicBlock>> successors();

  void set_returns(bool ret);
};

using CFG = std::vector<std::shared_ptr<BasicBlock>>;

} // namespace NL

#endif // _NL_CFG_H_
