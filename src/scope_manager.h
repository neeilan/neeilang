#ifndef _NL_SCOPE_MANAGER_H_
#define _NL_SCOPE_MANAGER_H_

#include <cstddef> // for size_t

#include "scope.h"

struct ScopeManager {
  std::vector<Scope> scopes;
  std::size_t curr_scope = 0;
  std::size_t next_id = 0;

  explicit ScopeManager() {
    // Create the 'global' scope, with id 0.
    scopes.push_back(Scope(next_id++));
  }

  Scope current() { return scopes[curr_scope]; }

  Scope globals() { return scopes[0]; }

  void enter() {
    Scope new_scope = current().create_child(next_id++);
    scopes.push_back(new_scope);
    curr_scope = new_scope.id;
  }

  void exit() { curr_scope = current().parent; }
};

#endif // _NL_SCOPE_MANAGER_H_