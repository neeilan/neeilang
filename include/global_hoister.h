#ifndef _NL_GLOBAL_HOISTER_H_
#define _NL_GLOBAL_HOISTER_H_

#include <string>
#include <map>
#include <vector>

#include "visitor.h"
#include "type_table.h"
#include "primitives.h"

/*
 * GlobalHoister helps implement hoist semantics for top-level
 * classes. Because NL clases can't be nested, every available
 * type in a program is either a primitive or a (global) class.
 * Hoisting enables types to be used anywhere in the program,
 * even in mutually-recursive type definitions.
 */
class GlobalHoister : public StmtVisitor<void>
{
  public:
    GlobalHoister() {
      typetab.insert("String", Primitives::String());
      typetab.insert("Int", Primitives::Int());
      typetab.insert("Float", Primitives::Float());
      typetab.insert("Bool", Primitives::Bool());
      typetab.insert("Void", Primitives::Void());
    }

    void hoist(const std::vector<Stmt *> statements);
    void hoist(const Stmt * stmt);
    void visit(const BlockStmt *);
    void visit(const ExprStmt *);
    void visit(const PrintStmt *);
    void visit(const VarStmt *);
    void visit(const ClassStmt *);
    void visit(const IfStmt *);
    void visit(const WhileStmt *);
    void visit(const FuncStmt *);
    void visit(const ReturnStmt *);

    TypeTable & get_type_table();

  private:
    void declare(const std::string & type_name);
    TypeTable typetab;
    bool decl_only_pass;
};

#endif //_NL_GLOBAL_HOISTER_H_
