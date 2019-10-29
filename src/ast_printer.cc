#include "ast_printer.h"
#include "stmt.h"
#include "expr.h"

#include <sstream>

using std::ostringstream;

std::string AstPrinter::visit(const BlockStmt *stmt) {
  ostringstream out;
  out << "(BLOCKSTMT " << std::endl;
  for (Stmt *inner_stmt : stmt->block_contents) {
    out << "  " << print(*inner_stmt) << std::endl;
  }
  out << ")";

  return out.str();
}

std::string AstPrinter::visit(const ExprStmt *stmt) {
  return print(*stmt->expression);
}

std::string AstPrinter::visit(const PrintStmt *stmt) {
  return "(PRINTSTMT " + print(*(stmt->expression)) + ")";
}

std::string AstPrinter::visit(const VarStmt *stmt) {
  ostringstream out;
  out << "(VARSTMT name=" << stmt->name.lexeme << " type=" << stmt->type.lexeme
      << " initializer = " << print(*stmt->expression) << ")";
  return out.str();
}

std::string AstPrinter::visit(const ClassStmt *stmt) {
  ostringstream out;
  out << "(CLASS " << stmt->name.str() << std::endl;
  if (stmt->superclass) {
    out << "  superclass: " << stmt->superclass->lexeme << std::endl;
  }
  out << "  fields: ";

  for (int i = 0; i < stmt->fields.size(); i++) {
    out << stmt->fields[i].lexeme << " type : " << stmt->field_types[i].lexeme;
    out << " ";
  }

  out << std::endl << "  methods:";
  for (int i = 0; i < stmt->methods.size(); i++) {
    out << "  " << stmt->methods[i]->accept(this);
  }

  out << std::endl << ")";
  return out.str();
}

std::string AstPrinter::visit(const IfStmt *stmt) {
  return "<BLOCK>";
  // return "if (" + visit(stmt->condition) + ") { " + visit(stmt->then_branch)
  // + "} else {" + print(stmt->else_branch) + "}":
}

std::string AstPrinter::visit(const WhileStmt *stmt) {
  return "<BLOCK>";
  // return "while (" + visit(stmt->condition) + ") { " + visit(stmt->body) +
  // "}";
}

std::string AstPrinter::visit(const FuncStmt *stmt) {
  ostringstream out;
  out << "(FUNCTION NAME=(" << stmt->name.str() << ")"
      << "  RETURNTYPE=(" << stmt->return_type.str() << ")"
      << "  ARGS=(";

  for (int i = 0; i < stmt->parameters.size(); i++) {
    out << stmt->parameters[i].str() << ": " << stmt->parameter_types[i].str();
  }
  out << "))";

  return out.str();
}

std::string AstPrinter::visit(const ReturnStmt *stmt) {
  return "<BLOCK>";
  //  return "return (" + print(stmt->value) + ")";
}

std::string AstPrinter::visit(const Binary *expr) {
  return parenthesize((expr->op).lexeme, &(expr->left), &(expr->right));
}

std::string AstPrinter::visit(const Call *expr) { return "Call"; }

std::string AstPrinter::visit(const Get *expr) { return "Get"; }

std::string AstPrinter::visit(const Set *expr) { return "Set"; }

std::string AstPrinter::visit(const This *expr) { return "This"; }

std::string AstPrinter::visit(const Assignment *expr) { return "Assignment"; }

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
    return std::to_string(expr->value);
  }
}

std::string AstPrinter::visit(const BoolLiteral *expr) {
  if (expr->value) {
    return "TRUE";
  } else {
    return "FALSE";
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
  return "(" + name + " " + expr->accept(this) + ")";
}

std::string AstPrinter::parenthesize(std::string name, const Expr *expr1,
                                     const Expr *expr2) {
  return "(" + name + " " + expr1->accept(this) + " " + expr2->accept(this) +
         ")";
}