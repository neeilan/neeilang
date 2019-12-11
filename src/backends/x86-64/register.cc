#include "backends/x86-64/register.h"

#include <set>
#include <string>

Register NO_REGISTER_AVAILABLE("no_reg_available");

// These are the 64-bit registers that are callee-saved (preserved across
// syscalls). See page 21 of https://www.uclibc.org/docs/psABI-x86_64.pdf
static std::set<std::string> unallocated = {"r12", "r13", "r14", "r15"};

Register allocate_reg64() {
  // Allocates a general-purpose 64-bit register
  if (unallocated.size() == 0) {
    return NO_REGISTER_AVAILABLE; // TODO: Implement spilling.
  }

  for (auto it = unallocated.begin();;) {
    Register r(*it);
    unallocated.erase(it);
    return r;
  }
}

void Register::free() const { unallocated.insert(name); }
