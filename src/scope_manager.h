#ifndef _NL_SCOPE_MANAGER_H_
#define _NL_SCOPE_MANAGER_H_

#include <cstddef> // for size_t

#include "scope.h"

struct ScopeManager {
  std::vector<Scope> scopes;
  std::size_t curr_scope = 0;
  std::size_t next_id = 0;

  ScopeManager() {
    // Create the 'global' scope, with id 0.
    scopes.push_back(Scope(next_id++));
  }

  Scope current() {
    return scopes[curr_scope];
  }


  Scope globals() {
    return scopes[0];
  } 

  Scope enter() {
    Scope new_scope = current().create_child(next_id++);
    scopes.push_back(new_scope);
    curr_scope = new_scope.id;
    return new_scope; 
  }

  Scope exit() {
    if (curr_scope == 0) {
      throw "Attempting to exit from global scope";
    }

    Scope prev_scope = *current().parent;
    curr_scope = prev_scope.id;
    return prev_scope;
  }
};

#endif  // _NL_SCOPE_MANAGER_H_
