#ifndef _NL_GLOBAL_HOISTER_H_
#define _NL_GLOBAL_HOISTER_H_

#include <map>
#include <string>
#include <vector>

#include "primitives.h"
#include "scope_manager.h"
#include "visitor.h"

/*
 * GlobalHoister helps implement hoist semantics for top-level
 * classes. Because NL clases can't be nested, every available
 * type in a program is either a primitive or a (global) class.
 * Hoisting enables types to be used anywhere in the program,
 * even in mutually-recursive type definitions.
 */
class GlobalHoister : public StmtVisitor<void> {
public:
  GlobalHoister(ScopeManager &sm) : sm(sm) {
    typetab()->insert("String", Primitives::String());
    typetab()->insert("Int", Primitives::Int());
    typetab()->insert("Float", Primitives::Float());
    typetab()->insert("Bool", Primitives::Bool());
    typetab()->insert("Void", Primitives::Void());
  }

  void hoist(const std::vector<Stmt *> statements);
  void hoist(const Stmt *stmt);

  STMT_VISITOR_METHODS(void)

private:
  ScopeManager sm;
  bool decl_only_pass;

  void declare(const std::string &type_name);

  std::shared_ptr<TypeTable> typetab() { return sm.current().typetab; }
};

#endif //_NL_GLOBAL_HOISTER_H_
