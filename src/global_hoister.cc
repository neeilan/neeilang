#include <string>
#include <map>
#include <vector>

#include "neeilang.h"
#include "global_hoister.h"
#include "type.h"
#include "stmt.h"

void GlobalHoister::declare(const std::string & type_name) {
  typetab.insert(type_name, std::make_shared<Type>(type_name));
}

void GlobalHoister::hoist(const std::vector<Stmt *> statements) {
  decl_only_pass = true;
  for (const Stmt * stmt : statements) {
    hoist(stmt);
  }

  decl_only_pass = false;
  for (const Stmt * stmt : statements) {
    hoist(stmt);
  }
}

void GlobalHoister::hoist(const Stmt * stmt) {
  stmt->accept(this);
  
}

void GlobalHoister::visit(const ClassStmt * cls) {
  const std::string cls_name = cls->name.lexeme;

  if (decl_only_pass) {
    declare(cls_name); // store a pointer to this type to hoist later
    return;
  }


  std::shared_ptr<Type> cls_type;
  if (typetab.contains(cls_name)) {
    cls_type = typetab.get(cls_name);
  } else {
    cls_type = std::make_shared<Type>(cls_name);
    typetab.insert(cls_name, cls_type);
  }

  // Supertype
  if (cls->superclass) {
    std::string supercls_name = cls->superclass->lexeme;
    if (!typetab.contains(supercls_name)) {
      Neeilang::error(*cls->superclass, "Unknown superclass");
      return;
    }

    std::shared_ptr<Type> supercls = typetab.get(supercls_name);

    // Circular inheritance is an error.
    if (supercls->subclass_of(cls_type.get())) {
      Neeilang::error(*cls->superclass, "Cycle in class hierarchy");
      return;
    }

    cls_type->supertype = supercls;
  }

  // Fields
  for (int i = 0; i < cls->fields.size(); i++) {
    std::string field_name = cls->fields[i].lexeme;
    std::string field_type_name = cls->field_types[i].lexeme;

    if (!typetab.contains(field_type_name)) {
      Neeilang::error(cls->field_types[i], "Unknown field type");
      return;
    }

    cls_type->fields.push_back( Field { field_name, typetab.get(field_type_name) });
  }
}


void GlobalHoister::visit(const FuncStmt * stmt) {}
void GlobalHoister::visit(const BlockStmt * stmt) {}
void GlobalHoister::visit(const ExprStmt * stmt) {}
void GlobalHoister::visit(const PrintStmt * stmt) {}
void GlobalHoister::visit(const VarStmt * stmt) {}
void GlobalHoister::visit(const IfStmt * stmt) {}
void GlobalHoister::visit(const WhileStmt * stmt) {}
void GlobalHoister::visit(const ReturnStmt * stmt) {}

