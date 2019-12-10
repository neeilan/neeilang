#include <map>
#include <string>
#include <vector>

#include "functype.h"
#include "global-hoister.h"
#include "neeilang.h"
#include "stmt.h"
#include "symtab.h"
#include "type.h"

static int arr_depth(const std::string &type) {
  int depth = 0;
  while (type[depth] == '[')
    depth++;
  return depth;
}

static std::string element_type(const std::string &type) {
  int depth = arr_depth(type);
  return type.substr(depth, type.size() - 2 * depth);
}

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
  for (int i = 0; i < cls->fields.size(); i++) {
    std::string field_name = cls->fields[i].lexeme;
    std::string field_type_name = cls->field_types[i].lexeme;

    if (!typetab()->contains(field_type_name)) {
      Neeilang::error(cls->field_types[i], "Unknown field type");
      return;
    }

    cls_type->fields.push_back(
        Field{field_name, typetab()->get(field_type_name)});
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

  if (typetab()->contains(element_type(type))) {
    declare(type);
    auto arr_type = typetab()->get(type);
    arr_type->element_type = typetab()->get(element_type(type));
    arr_type->arr_depth = arr_depth(type);
  }
}

void GlobalHoister::visit(const FuncStmt *stmt) {
  // Need types to be declared in first pass, since they
  // may be used in the function.
  if (decl_only_pass) {
    return;
  }

  const std::string fn_name = stmt->name.lexeme;
  const std::string return_type_name = stmt->return_type.lexeme;
  hoist_type(return_type_name);

  std::shared_ptr<FuncType> functype = std::make_shared<FuncType>();
  functype->name = fn_name; // TODO: Constructor this.

  bool had_error = false;

  if (!typetab()->contains(stmt->return_type.lexeme)) {
    Neeilang::error(stmt->return_type,
                    "Unknown return type " + stmt->return_type.lexeme);
    had_error = true;
  } else {
    functype->return_type = typetab()->get(stmt->return_type.lexeme);
  }

  for (Token param_type : stmt->parameter_types) {
    hoist_type(param_type.lexeme);
    if (!typetab()->contains(param_type.lexeme)) {
      Neeilang::error(param_type, "Unknown parameter type");
      had_error = true;
    } else {
      functype->arg_types.push_back(typetab()->get(param_type.lexeme));
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
  if (decl_only_pass)
    return;
  const std::string type = stmt->type.lexeme;
  hoist_type(type);
  if (!typetab()->contains(type)) {
    Neeilang::error(stmt->type, "Unknown type in variable declaration.");
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
