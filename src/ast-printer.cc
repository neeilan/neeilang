#include "ast-printer.h"
#include "expr.h"
#include "stmt.h"

#include <sstream>

using std::ostringstream;

static int nest = 0;

#define OUT out << std::string(nest*2, ' ')

std::string AstPrinter::print(const std::vector<Stmt *> &program) {
  ostringstream out;
  for (const Stmt *stmt : program) {
    out << print(stmt) << std::endl;
  }
  return out.str();
}

std::string AstPrinter::visit(const NamespaceStmt *stmt) {
  ostringstream out;
  std::string name = "(anonymous)";
  if (!stmt->name.empty()) { name = stmt->name; }
  OUT << "<Namespace " << name << ">\n";
  nest++;
  out << print(stmt->contents);
  nest--;
  OUT << "</Namespace " << name << ">";
  return out.str();
}

std::string AstPrinter::visit(const ScopedEnum * stmt) {
  ostringstream out;
  OUT << "<ScopedEnum name=\"" << stmt->name << "\" underlying=\""
    << (stmt->underlying ? stmt->underlying->name.str() : "(nullopt)")
    << "\">\n";

  nest++;
  for (const auto& val : stmt->enumerators) {
    OUT << "<NamedEnumerator name=\"" << val.name.lexeme << "\" value=\""
        << (val.value ? val.value->lexeme : "(nullopt)") << "\"/>\n";
  }
  nest--;
  OUT << "</ScopedEnum>";
  return out.str();
}

std::string AstPrinter::visit(const TemplateStmt * stmt) {
  ostringstream out;
  OUT  << "<Template [";
  for (auto const& arg : stmt->args) {
    out << "typename " << arg.name.lexeme;
    if (&arg != &stmt->args.back()) {
      out << ", ";
    }
  }
  out << "]>\n";
  nest++;
  out << print(stmt->fnOrClass);
  nest--;
  OUT << "</Template>";
  return out.str();
}

std::string AstPrinter::visit(const BlockStmt *stmt) {
  ostringstream out;
  if (!stmt->block_contents.size()) {
    OUT << "<Block/>\n";
    return out.str();
  }
  OUT << "<Block>\n";
  nest++;
  out << print(stmt->block_contents);
  nest--;
  OUT << "</Block>\n";
  return out.str();
}

std::string AstPrinter::visit(const ExprStmt *stmt) {
  ostringstream out;
  OUT << "<Expr " + print(stmt->expression) + "/>";
  return out.str();
}

std::string AstPrinter::visit(const PrintStmt *stmt) {
  ostringstream out;
  OUT << "<Print " + print(stmt->expression) + "/>";
  return out.str();
}

std::string AstPrinter::visit(const VarStmt *stmt) {
  ostringstream out;
  OUT << "<Var name=" << stmt->name.lexeme << " type=" << stmt->tp.name.str();
  if (stmt->expression) {
    out << " initializer=" << print(stmt->expression);
  }
  out << ">";

  return out.str();
}

std::string AstPrinter::visit(const ClassStmt *stmt) {
  ostringstream out;
  OUT << "<Class " << stmt->name.lexeme;
  if (stmt->superclass) {
    out << "  superclass=" << stmt->superclass->lexeme << std::endl;
  }
  out << ">\n";
  nest++;
  OUT << "<Class.Fields>" << std::endl;
  nest++;
  for (size_t i = 0; i < stmt->fields.size(); i++) {
    OUT << "<Field name=\"" << stmt->fields[i].lexeme
        << "\" type=\"" << stmt->field_types[i].name.str() << "\"/>\n";
  }
  nest--;
  OUT << "</Class.Fields>" << std::endl;
  OUT << "<Class.Methods> " << std::endl;
  nest++;
  for (size_t i = 0; i < stmt->methods.size(); i++) {
    out << print(stmt->methods[i]);
  }
  nest--;
  OUT << "</Class.Methods>\n";
  nest--;
  OUT << "</Class>\n";
  return out.str();
}

std::string AstPrinter::visit(const IfStmt *stmt) {
  ostringstream out;
  OUT << "<IfStmt condition=";
  if (stmt->condition) {
    out << print(stmt->condition);
  }
  OUT << ">\n";
  if (stmt->then_branch) {
    nest++;
    OUT << "<Then>\n";
    nest++;
    out << print(stmt->then_branch);
    nest--;
    OUT << "</Then>\n";
    nest--;
  }
  if (stmt->else_branch) {
    nest++;
    OUT << "<Else>\n";
    nest++;
    out << print(stmt->else_branch);
    nest--;
    OUT << "</Else>\n";
    nest--;
  }
  return out.str();
}

std::string AstPrinter::visit(const WhileStmt *stmt) {
  ostringstream out;
  OUT << "<While condition=";
  if (stmt->condition) {
    out << print(stmt->condition);
  }
  if (stmt->body) {
    OUT << "body:" << std::endl;
    nest++;
    out << print(stmt->body) << std::endl;
    nest--;
  }
  return out.str();
}

std::string AstPrinter::visit(const FuncStmt *stmt) {
  ostringstream out;
  OUT << "<Function name=\"" << stmt->name.lexeme << "\" specifiers=\"" << stmt->specifiers.str() << "\"  return_type=\""
      << stmt->return_type.name.str() << "\" ";
  for (size_t i = 0; i < stmt->parameters.size(); i++) {
    std::string argn = std::string("args[") + std::to_string(i) + "]";
    out << argn << ".name=\"" << stmt->parameters[i].lexeme << "\" "
        << argn << ".type=\"" << stmt->parameter_types[i].name.str() << "\" ";
  }
  out << ">\n";

  nest++;
  for (auto _stmt : stmt->body) {
    out << print(_stmt);
  }
  nest--;

  OUT << "</Function>\n";
  return out.str();
}

std::string AstPrinter::visit(const ReturnStmt *stmt) {
  ostringstream out;
  if (!stmt->value)
    OUT << "<Return (void)>";
  else
    OUT << "<Return (" + print(stmt->value) + ")>";
  return out.str();
}

std::string AstPrinter::visit(const Binary *expr) {
  return parenthesize((expr->op).lexeme, &(expr->left), &(expr->right));
}

std::string AstPrinter::visit(const Call *expr) { return "Call"; }

std::string AstPrinter::visit(const Get *expr) { return "Get " + expr->name.lexeme; }

std::string AstPrinter::visit(const Set *expr) { return "Set " + expr->name.lexeme; }

std::string AstPrinter::visit(const GetIndex *expr) {
  return "<GetIndex " + print(&expr->callee) + "[" + print(&expr->index) + "]>";
}

std::string AstPrinter::visit(const SetIndex *expr) {
  return "<SetIndex " + print(&expr->callee) + "[" + print(&expr->index) +
         "] = " + print(&expr->value) + ">";
}

std::string AstPrinter::visit(const This *expr) { return "This"; }

std::string AstPrinter::visit(const Assignment *expr) {
  return "<Assignment var=" + expr->name.lexeme +
         " value=" + print(&expr->value) + ">";
}

std::string AstPrinter::visit(const StrLiteral *expr) {
  if (expr->nil) {
    return "nil";
  } else {
    return "\"" + expr->value + "\"";
  }
}

std::string AstPrinter::visit(const NumLiteral *expr) {
  if (expr->nil) {
    return "nil";
  } else {
    return expr->value;
  }
}

std::string AstPrinter::visit(const BoolLiteral *expr) {
  if (expr->value) {
    return "True";
  } else {
    return "False";
  }
}

std::string AstPrinter::visit(const Grouping *expr) {
  return parenthesize(std::string("group"), &(expr->expression));
}

std::string AstPrinter::visit(const Unary *expr) {
  return parenthesize((expr->op).lexeme, &(expr->right));
}

std::string AstPrinter::visit(const Variable *expr) {
  return expr->name.lexeme;
}

std::string AstPrinter::visit(const Logical *expr) {
  return parenthesize(expr->op.lexeme, &expr->left, &expr->right);
}

std::string AstPrinter::parenthesize(std::string name, const Expr *expr) {
  return "(" + name + " " + print(expr) + ")";
}

std::string AstPrinter::parenthesize(std::string name, const Expr *expr1,
                                     const Expr *expr2) {
  return "(" + name + " " + print(expr1) + " " + print(expr2) + ")";
}

std::string AstPrinter::visit(const SentinelExpr *expr) {
  return "<SentinelExpr>";
}
