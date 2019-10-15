#ifndef _NL_SCOPE_TRACKER_H_
#define _NL_SCOPE_TRACKER_H_

#include <cstddef> // for size_t

struct ScopeTracker {
  std::size_t id = 0;
  int advance() {
    return ++id;
  }
};

#endif  // _NL_SCOPE_TRACKER_H_
