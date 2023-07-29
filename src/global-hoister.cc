#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "functype.h"
#include "global-hoister.h"
#include "neeilang.h"
#include "stmt.h"
#include "symtab.h"
#include "type.h"

void GlobalHoister::declare(const std::string &type_name) {
  typetab()->insert(type_name, std::make_shared<Type>(type_name));
}

void GlobalHoister::hoist_program(const std::vector<Stmt *> statements) {
  decl_only_pass = true;
  hoist(statements);

  decl_only_pass = false;
  hoist(statements);
}

void GlobalHoister::hoist(const std::vector<Stmt *> statements) {
  for (const Stmt *stmt : statements) {
    hoist(stmt);
  }
}

void GlobalHoister::hoist(const Stmt *stmt) { stmt->accept(this); }

void GlobalHoister::visit(const ClassStmt *cls) {
  const std::string cls_name = cls->name.lexeme;

  if (decl_only_pass) {
    declare(cls_name); // store a pointer to this type to hoist later
    return;
  }

  // Insert a symbol (of Class type)
  Symbol symbol{cls_name, Primitives::Class()};
  symtab()->insert(cls_name, symbol);

  NLType cls_type;
  if (typetab()->contains(cls_name)) {
    cls_type = typetab()->get(cls_name);
  } else {
    cls_type = std::make_shared<Type>(cls_name);
    typetab()->insert(cls_name, cls_type);
  }

  // Supertype
  if (cls->superclass) {
    std::string supercls_name = cls->superclass->lexeme;
    if (!typetab()->contains(supercls_name)) {
      Neeilang::error(*cls->superclass, "Unknown superclass");
      return;
    }

    NLType supercls = typetab()->get(supercls_name);

    // Circular inheritance is an error.
    if (supercls->subclass_of(cls_type.get())) {
      Neeilang::error(*cls->superclass, "Cycle in class hierarchy");
      return;
    }

    cls_type->supertype = supercls;
  }

  // Fields
  for (size_t i = 0; i < cls->fields.size(); i++) {
    std::string field_name = cls->fields[i].lexeme;
    std::string field_type_name = cls->field_types[i].name.lexeme;

    if (!typetab()->contains(field_type_name)) {
      Neeilang::error(cls->field_types[i].name, "Unknown type in field");
      return;
    }

    NLType field_type = typetab()->get(field_type_name);

    TypeParse field_tp = cls->field_types[i];
    if (field_tp.is_array()) {
      field_type = Primitives::Array(field_type, field_tp.array_dims());
    }

    cls_type->fields.push_back(Field{field_name, field_type});
  }

  // Methods
  NLType old_encl_class = encl_class;
  encl_class = cls_type;
  for (const Stmt *method : cls->methods) {
    hoist(method);
  }
  encl_class = old_encl_class;
}

void GlobalHoister::hoist_type(const std::string &type) {
  if (typetab()->contains(type))
    return;

  // This could be an array type.
}

void GlobalHoister::visit(const FuncStmt *stmt) {
  // Need types to be declared in first pass, since they
  // may be used in the function.
  if (decl_only_pass) {
    return;
  }

  const std::string fn_name = stmt->name.lexeme;
  const std::string return_type_name = stmt->return_type.name.lexeme;
  hoist_type(return_type_name);

  std::shared_ptr<FuncType> functype = std::make_shared<FuncType>();
  functype->name = fn_name; // TODO: Constructor this.

  bool had_error = false;

  if (!typetab()->contains(stmt->return_type.name.lexeme)) {
    Neeilang::error(stmt->return_type.name,
                    "Unknown return type " + stmt->return_type.name.lexeme);
    had_error = true;
  } else {
    functype->return_type = typetab()->get(stmt->return_type.name.lexeme);
    if (stmt->return_type.is_array()) {
      functype->return_type = Primitives::Array(functype->return_type,
                                                stmt->return_type.array_dims());
    }
  }

  for (TypeParse param_tp : stmt->parameter_types) {
    hoist_type(param_tp.name.lexeme);
    if (!typetab()->contains(param_tp.name.lexeme)) {
      Neeilang::error(param_tp.name, "Unknown parameter type");
      had_error = true;
    } else {
      NLType param_type = typetab()->get(param_tp.name.lexeme);
      if (param_tp.is_array()) {
        param_type = Primitives::Array(param_type, param_tp.array_dims());
      }
      functype->arg_types.push_back(param_type);
    }
  }

  if (had_error) {
    return;
  }

  if (encl_class) {
    // Method
    encl_class->methods.push_back(functype);
  } else {
    // Regular (global) function
    const std::string fn_key = TypeTableUtil::fn_key(stmt);
    declare(fn_key);
    typetab()->get(fn_key)->functype = functype;
  }

  hoist(stmt->body);
}

void GlobalHoister::visit(const BlockStmt *stmt) {
  if (decl_only_pass)
    return;
  hoist(stmt->block_contents);
}

void GlobalHoister::visit(const VarStmt *stmt) {
  if (decl_only_pass) {
    return;
  }
  if (stmt->tp.inferred) {
    return;
  }

  const std::string type = stmt->tp.name.lexeme;

  hoist_type(type);
  if (!typetab()->contains(type)) {
    Neeilang::error(stmt->tp.name, "Unknown type in variable declaration.");
  }
}

void GlobalHoister::visit(const WhileStmt *stmt) {
  if (decl_only_pass)
    return;
  if (stmt->body)
    hoist(stmt->body);
}

void GlobalHoister::visit(const IfStmt *stmt) {
  if (decl_only_pass)
    return;
  if (stmt->then_branch)
    hoist(stmt->then_branch);
  if (stmt->else_branch)
    hoist(stmt->else_branch);
}

// These statements cannot introduce new types.
void GlobalHoister::visit(const ExprStmt *stmt) {}
void GlobalHoister::visit(const PrintStmt *stmt) {}
void GlobalHoister::visit(const ReturnStmt *stmt) {}
