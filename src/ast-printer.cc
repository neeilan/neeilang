#include "ast-printer.h"

#include <sstream>

using std::ostringstream;

static int nest = 0;

#define OUT out << std::string(nest*2, ' ')

std::string AstPrinter::print(const std::vector<const Stmt *> &program) {
  ostringstream out;
  for (const Stmt *stmt : program) {
    out << print(stmt) << std::endl;
  }
  return out.str();
}

std::string AstPrinter::printTypeParse(TypeParse const & tc) {
  TypeParse& t = const_cast<TypeParse&>(tc);
  if (!t.declTypeExpr) {
    return t.prettyName();
  }
  std::string declTypeDesc = "decltype(";
  declTypeDesc += print(t.declTypeExpr);
  declTypeDesc += ')';
  t.declTypeDesc = declTypeDesc;
  return t.prettyName();
}


std::string AstPrinter::visit(const NamespaceStmt *stmt) {
  ostringstream out;
  std::string name = "(anonymous)";
  if (!stmt->name.empty()) { name = stmt->name; }
  OUT << "<Namespace declctx=\"" << stmt->ctx->name() << "\" name=\"" << name << "\">\n";
  nest++;
  out << print(stmt->contents);
  nest--;
  OUT << "</Namespace " << name << ">";
  return out.str();
}

std::string AstPrinter::visit(const ScopedEnum * stmt) {
  ostringstream out;
  OUT << "<ScopedEnum name=\"" << stmt->name
      << "\" declctx=\"" << stmt->ctx->name()
      << "\" underlying=\""
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
    out << "typename "
        << (arg.isVariadic ? "..." : "")
        << arg.name.lexeme;
    if (&arg != &stmt->args.back()) {
      out << ", ";
    }
  }
  out << "]>\n";
  nest++;
  out << print(stmt->decl);
  nest--;
  OUT << "</Template>";
  return out.str();
}

std::string AstPrinter::visit(const StaticAssertStmt * stmt) {
  ostringstream out;
  OUT  << "<StaticAssert>";
  nest++;
  out << print(stmt->value);
  nest--;
  OUT << "</StaticAssert>";
  return out.str();
}


std::string AstPrinter::visit(const UsingStmt * stmt) {
  ostringstream out;
  const char* nsPart = stmt->variant.isNamespace ? " namespace" : "";
  const char* enumPart = stmt->variant.isEnum ? " enum" : "";
  OUT  << "<Using" << nsPart << enumPart << " name=\"" << stmt->name.str() << "\"/>";
  return out.str();
}

std::string AstPrinter::visit(const AliasStmt * stmt) {
  ostringstream out;
  OUT  << "<TypeAlias origId=\"" << stmt->origId.str() << "\" alias=\""<< stmt->alias.str() << "\"/>";
  return out.str();
}

std::string AstPrinter::visit(const BlockStmt *stmt) {
  ostringstream out;
  OUT << "<Block declctx=\"" << stmt->ctx->name() << "\">\n";
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
  OUT << "<Var name=\"" << stmt->name.lexeme << "\" type=\"" << printTypeParse(stmt->tp) << "\">\n";
  if (stmt->expression) {
    nest++;
    OUT << "<Var.Initializer>\n";
    nest++;
    OUT << print(stmt->expression) << '\n';
    nest--;
    OUT << "</Var.Initializer>\n";
    nest--;
  }
  OUT << "</Var>";

  return out.str();
}

std::string AstPrinter::visit(const ClassStmt *stmt) {
  ostringstream out;
  OUT << "<Class name=\"" << stmt->name.lexeme
                   << "\" declctx=\"" << stmt->ctx->name() << "\"";
  if (stmt->superclass) {
    out << "  superclass=" << stmt->superclass->prettyName() << std::endl;
  }
  out << ">\n";
  nest++;
  OUT << "<Class.MemberDecls>" << std::endl;
  nest++;
  for (size_t i = 0; i < stmt->memberDecls.size(); i++) {
    out << print(stmt->memberDecls[i]) << std::endl;
  }
  nest--;
  OUT << "</Class.MemberDecls>\n";
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

  std::string name = stmt->name.lexeme;
  if (stmt->operatorOverload) {
    name += " ";
    name += getTokenTypeName(*stmt->operatorOverload);
  }
  OUT << "<Function name=\"" << name
    << "\" declctx=\"" << stmt->ctx->name()
    << "\" specifiers=\"" << stmt->specifiers.str()
    << "\"  return_type=\""
      << printTypeParse(stmt->return_type) << "\" ";
  for (size_t i = 0; i < stmt->parameters.size(); i++) {
    bool isVariadic = stmt->parameter_types[i].isVariadic;
    std::string argn = std::string("args[") + std::to_string(i) + "]";
    out << argn << ".name=\"" << stmt->parameters[i].lexeme << "\" "
        << argn << ".type=\"" << printTypeParse(stmt->parameter_types[i])
        << (isVariadic ? "..." : "") << "\" ";
    auto defIt = stmt->defaultArgs.find(i);
    if (defIt != stmt->defaultArgs.end()) {
      out  << argn << ".default=\"" << print(defIt->second)  << "\" ";
    }
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
  return parenthesize((expr->op).str(), &(expr->left), &(expr->right));
}

std::string AstPrinter::visit(const Call *expr) {
  ostringstream out;
  OUT << "<Call>\n";
  nest++;
  OUT << "<Call.callee>\n";
  nest++;
  OUT << print(&expr->callee) << '\n';
  nest--;
  OUT << "</Call.callee>\n";
  OUT << "<Call.args>\n";
  nest++;
  for (auto const* arg : expr->args) {
    OUT << print(arg) << '\n';
  }
  nest--;
  OUT << "</Call.args>\n";
  nest--;
  OUT << "</Call>";
  return out.str();
}

std::string AstPrinter::visit(const Get *expr) {
  return "(Get (" + print(&expr->callee) + ")::" + expr->name.lexeme + " via '" + (expr->accessOp == DOT ? "." : "->") +  "')";
}

std::string AstPrinter::visit(const Set *expr) {
  ostringstream out;
  OUT << "<Set field=\"" << expr->name.lexeme << "\" >";
  nest++;
  out << print(&expr->value);
  nest--;
  OUT << "</Set>";
  return out.str();
}

std::string AstPrinter::visit(const GetIndex *expr) {
  return "<GetIndex " + print(&expr->callee) + "[" + print(&expr->index) + "]>";
}

std::string AstPrinter::visit(const SetIndex *expr) {
  return "<SetIndex " + print(&expr->callee) + "[" + print(&expr->index) +
         "] = " + print(&expr->value) + ">";
}

std::string AstPrinter::visit(const This *expr) { return "This"; }

std::string AstPrinter::visit(const SizeOf *expr) {
  std::string operand;
  if (std::holds_alternative<TypeParse>(expr->operand)) {
    operand = "<type> " + printTypeParse(std::get<TypeParse>(expr->operand));
  } else {
    auto *opExp = std::get<Expr*>(expr->operand);
    operand = print(opExp);
  }
  return "(SizeOf operand=\"" + operand + "\")";
}

std::string AstPrinter::visit(const AlignOf *expr) {
  return "(AlignOf \"" + printTypeParse(expr->typeId) + "\")";
}

std::string AstPrinter::visit(const StaticCast *expr) {
  return "(StaticCast \""
    + printTypeParse(expr->typeId)
    + " expr=\""
    + print(expr->expr)
    + "\""
    + "\")";
}

std::string AstPrinter::visit(const Assignment *expr) {
  return "<Assignment var=" + expr->name.str() +
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
  ostringstream out;
  if (expr->nil) {
    return "nil";
  } else {
    out << "(NumLiteral value=\"" << expr->value << "\")";
  }
  return out.str();
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
  ostringstream out;
  out << "(Variable name=\"" << expr->name.str() << "\")";
  return out.str();
}

std::string AstPrinter::visit(const Logical *expr) {
  return parenthesize(expr->op.str(), &expr->left, &expr->right);
}

std::string AstPrinter::parenthesize(std::string name, const Expr *expr) {
  return "((" + name + ") " + print(expr) + ")";
}

std::string AstPrinter::parenthesize(std::string name, const Expr *expr1,
                                     const Expr *expr2) {
  return "((" + name + ") " + print(expr1) + " " + print(expr2) + ")";
}

std::string AstPrinter::visit(const SentinelExpr *expr) {
  return "<SentinelExpr>";
}
