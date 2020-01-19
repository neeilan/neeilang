#include "ast-printer.h"
#include "expr.h"
#include "stmt.h"

#include <sstream>

using std::ostringstream;

static int nest = 0;

#define OUT out << std::string(nest, ' ')

std::string AstPrinter::print(const std::vector<Stmt *> &program) {
  ostringstream out;
  for (const Stmt *stmt : program) {
    out << print(stmt) << std::endl;
  }
  return out.str();
}

std::string AstPrinter::visit(const BlockStmt *stmt) {
  ostringstream out;
  if (!stmt->block_contents.size()) {
    return "<Block>";
  }
  OUT << "<Block";
  out << std::endl;
  nest++;
  out << print(stmt->block_contents);
  nest--;
  OUT << ">" << std::endl;
  return out.str();
}

std::string AstPrinter::visit(const ExprStmt *stmt) {
  return std::string(nest, ' ') + print(stmt->expression);
}

std::string AstPrinter::visit(const PrintStmt *stmt) {
  return "<Print " + print(stmt->expression) + ">";
}

std::string AstPrinter::visit(const VarStmt *stmt) {
  ostringstream out;
  OUT << "<Var name=" << stmt->name.lexeme << " type=" << stmt->tp.name.lexeme;
  if (stmt->expression) {
    out << " initializer=" << print(stmt->expression);
  }
  out << ">";

  return out.str();
}

std::string AstPrinter::visit(const ClassStmt *stmt) {
  ostringstream out;
  OUT << "<Class " << stmt->name.lexeme << std::endl;
  if (stmt->superclass) {
    out << "  superclass=" << stmt->superclass->lexeme << std::endl;
  }
  nest++;
  OUT << "fields: " << std::endl;
  nest++;
  for (int i = 0; i < stmt->fields.size(); i++) {
    OUT << stmt->fields[i].lexeme
        << " type : " << stmt->field_types[i].name.lexeme;
    out << " ";
  }
  nest--;

  OUT << std::endl;
  OUT << "methods: " << std::endl;

  nest++;
  for (int i = 0; i < stmt->methods.size(); i++) {
    OUT << print(stmt->methods[i]);
  }
  nest--;
  OUT << ">";
  return out.str();
}

std::string AstPrinter::visit(const IfStmt *stmt) {
  ostringstream out;
  OUT << "<IfStmt condition=";
  if (stmt->condition) {
    out << print(stmt->condition);
  }
  if (stmt->then_branch) {
    OUT << "then: ";
    nest++;
    out << print(stmt->then_branch);
    nest--;
  }
  if (stmt->else_branch) {
    OUT << "else: ";
    nest++;
    out << print(stmt->else_branch);
    nest--;
  }
  nest--;
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
  out << "<Function name='" << stmt->name.lexeme << "'  returns='"
      << stmt->return_type.lexeme << "'"
      << "  args=( ";

  for (int i = 0; i < stmt->parameters.size(); i++) {
    OUT << stmt->parameters[i].lexeme << ":" << stmt->parameter_types[i].lexeme
        << " ";
  }
  out << ") Body=" << std::endl;

  nest++;
  for (auto _stmt : stmt->body) {
    out << print(_stmt);
  }
  nest--;

  out << ">";

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

std::string AstPrinter::visit(const Get *expr) { return "Get"; }

std::string AstPrinter::visit(const Set *expr) { return "Set"; }

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
    return expr->value;
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
