#include "cfg.h"

#include <memory>

void NL::BasicBlock::add_successor(std::shared_ptr<BasicBlock> suc) {
  _successors.push_back(suc);
}

bool NL::BasicBlock::returns_at_node() { return _returns; }

bool NL::BasicBlock::returns() {
  if (_returns)
    return true;
  if (_successors.empty())
    return false;

  for (auto succ : _successors) {
    if (!succ->returns())
      return false;
  }
  return true;
}

std::vector<std::shared_ptr<NL::BasicBlock>> NL::BasicBlock::successors() {
  return _successors;
}

bool NL::BasicBlock::is_void_func() { return _fn_entry && _is_void; }

void NL::BasicBlock::set_returns(bool ret) { _returns = ret; }
