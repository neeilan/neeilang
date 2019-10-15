#include <string>
#include <sstream>
#include <vector>

#include "neeilang.h"
#include "primitives.h"
#include "type_checker.h"
#include "type_table.h"

#include <iostream>

static std::map<const Expr *, std::shared_ptr<Type>> expr_types;

void TypeChecker::check(const std::vector<Stmt *> statements)
{
    for (const Stmt * stmt : statements)
    {
        check(stmt);
    }
}

void TypeChecker::check(const Stmt * stmt)
{
    stmt->accept(this);
}

std::shared_ptr<Type> TypeChecker::check(const Expr * expr)
{
    expr->accept(this);
    std::shared_ptr<Type> type = expr_types[expr];
    expr_types.erase(expr);
    return type;
}

void TypeChecker::visit(const BlockStmt * stmt)
{ 
    check(stmt->block_contents);
}

void TypeChecker::visit(const VarStmt * stmt)
{
    auto var_type = types.get(stmt->type.lexeme);
    if (!var_type) {
      Neeilang::error(stmt->type, "Unknown type");
    }

    if (stmt->expression != nullptr)
    {
      auto expr_type = check(stmt->expression);
      if (!expr_type->subclass_of(var_type.get())) {
        std::ostringstream msg;
        msg << "Illegal initialization of variable of type " << var_type->name
            << " with expression of type " << expr_type->name;
        Neeilang::error(stmt->type, msg.str());
      }
   }
}

void TypeChecker::visit(const Variable * expr)
{ 
  auto fn_key = TypeTableUtil::fn_key(expr->name.lexeme);
  if (types.contains(fn_key)) {
    expr_types[expr] = types.get(fn_key);
  }
}

void TypeChecker::visit(const Assignment * expr)
{ 
  /*
  auto right = check(&expr->value);
  auto left = check(expr->name); // the var being assigned to.

  if (has_type_error( { left, right } )) {
    expr_types[expr] = Primitives::TypeError();
    return;
  }

  if (!right->subclass_of(left.get()) {
    expr_types[expr] = Primitives::TypeError();
    return;
  }

  expr_types[expr] = left;
 */
}

void TypeChecker::visit(const This * expr)
{ /*
    if (current_class != ClassType::IN_CLASS)
    {
        Neeilang::error(expr->keyword, "Cannot use 'this' outside of a class.");
        return;
    }

    resolve_local(expr, expr->keyword);
  */
}

void TypeChecker::visit(const FuncStmt * stmt)
{ 
  // GlobalHoister does a majority of the work.
  // TODO : Perhaps move some of that logic here?
}

void TypeChecker::visit(const ClassStmt * stmt)
{
  /*
    ClassType enclosing_class = current_class;
    current_class = ClassType::IN_CLASS;

    declare(stmt->name);
    define(stmt->name);

    if (stmt->superclass)
    {
        resolve(stmt->superclass);
    }

    begin_scope();

     * Whenever 'this' is encountered in a method, it will resolve to a
     * "local variable” in an implicit scope just outside the method body.
    scopes.back()->insert(std::pair<std::string, bool>("this", true));

    for (const Stmt * method : stmt->methods)
    {
        FunctionType declaration = METHOD;

        const FuncStmt * method_fn = static_cast<const FuncStmt *>(method);

        if (method_fn->name.lexeme == "init")
        {
            declaration = INITIALIZER;
        }

        resolve_fn(declaration, method_fn);
    }

    end_scope();

    current_class = enclosing_class;
  */
}

// Statements
void TypeChecker::visit(const ExprStmt * stmt)
{
   check(stmt->expression);
}

void TypeChecker::visit(const IfStmt * stmt)
{ 
    auto cond_type = check(stmt->condition);
    if (cond_type != Primitives::Bool()) {
        Neeilang::error(stmt->keyword, "Condition must be of type Bool. Got: " + cond_type->name);
    }

    if (stmt->then_branch) check(stmt->then_branch);
    if (stmt->else_branch) check(stmt->else_branch);
}

void TypeChecker::visit(const PrintStmt * stmt)
{
    auto expr_type = check(stmt->expression);
    if (!match(expr_type, { Primitives::String() })) {
        Neeilang::error(stmt->keyword, "Expression to be printed must be a String. Got: " + expr_type->name);
    }
}

void TypeChecker::visit(const ReturnStmt * stmt)
{
    /*
    if (current_function == NOT_IN_FN)
    {
        Neeilang::error(stmt->keyword, "Cannot return from top-level code.");
    }

    if (current_function == INITIALIZER && stmt->value)
    {
        Neeilang::error(stmt->keyword, "Cannot return a value from initializer.");
    }

    if (stmt->value)
    {
        resolve(stmt->value);
    }
   */
}

void TypeChecker::visit(const WhileStmt * stmt)
{
    auto condition_type = check(stmt->condition);
    if (condition_type != Primitives::Bool()) {
      Neeilang::error(stmt->while_tok, "Loop condition must be of type Bool");
    }
    check(stmt->body);
}

// Expressions

void TypeChecker::visit(const Binary * expr)
{
    std::shared_ptr<Type> left = check(&expr->left);
    std::shared_ptr<Type> right = check(&expr->right);

    if (has_type_error( { left, right } )) {
      expr_types[expr] = Primitives::TypeError();
      return;
    }

    // TODO : Handle <, > operators.
    switch (expr->op.type) {
      case EQUAL_EQUAL:
      case BANG_EQUAL: {
        expr_types[expr] = Primitives::Bool();
        return;
      }
      case GREATER:
      case GREATER_EQUAL:
      case LESS:
      case LESS_EQUAL:
      case MINUS:
      case SLASH:
      case STAR: {
        if (match(left, { Primitives::Int(), Primitives::Float() })
            && match(right, { Primitives::Int(), Primitives::Float() })) {
          expr_types[expr] = Primitives::Bool();
          return;
        } else {
          Neeilang::error(expr->op, "Left and right operands must be numbers");
          expr_types[expr] = Primitives::TypeError();
          return;
        }
      }
      case PLUS: {
        if (left == Primitives::Int() && right == Primitives::Int()) {
          expr_types[expr] = Primitives::Int();
          return;
        } else if (left == Primitives::Float() && right == Primitives::Float()) {
          expr_types[expr] = Primitives::Float();
          return;
        }  else if ((left == Primitives::Float() || right == Primitives::Float()) &&
                    (left == Primitives::Int() || right == Primitives::Int()) ) {
          expr_types[expr] = Primitives::Float();
          return;
        } else if (left == Primitives::String() && right == Primitives::String()) {
          expr_types[expr] = Primitives::String();
          return;
        }

        Neeilang::error(expr->op, "Left and right operands must be numbers or Strings");
        expr_types[expr] = Primitives::TypeError();
        return;
      }
      default: {
        // Unreachable.
        expr_types[expr] = Primitives::TypeError();
        return;
      }
    }
}

void TypeChecker::visit(const Call * expr)
{
    auto callee_type = check(&expr->callee);
    if (!callee_type->functype) {
        Neeilang::error(expr->paren, "Expression is not callable");
        expr_types[expr] = Primitives::TypeError();
        return;
    }

    std::vector<shared_ptr<Type>> arg_types;
    for (Expr * arg : expr->args) {
      arg_types.push_back(check(arg));
    }

    auto functype = callee_type->functype;
    if (arg_types.size() != functype->arg_types.size()) {
        std::ostringstream msg;
        msg << "Expected " << functype->arg_types.size()
            << " args, but got " << arg_types.size(); 
        Neeilang::error(expr->paren, msg.str());
        expr_types[expr] = Primitives::TypeError();
        return;
    }

    if (!functype->accepts_args(arg_types)) {
        std::ostringstream msg;
        msg << "Expected argument type ( ";
        for (auto arg_type : functype->arg_types) {
          msg << arg_type->name << " ";
        }
        msg << ") but got ( ";
        for (auto arg_type : arg_types) {
          msg << arg_type->name << " ";
        }
        msg << ")";
      
        Neeilang::error(expr->paren, msg.str());
        expr_types[expr] = Primitives::TypeError();
        return;
    }

    expr_types[expr] = functype->return_type;
}

void TypeChecker::visit(const Get * expr)
{
    auto callee_type = check(&expr->callee);
    if ( has_type_error({ callee_type }) ) {
      expr_types[expr] = Primitives::TypeError();
      return;
    }

    auto field_name = expr->name.lexeme;
    if (!callee_type->has_field(field_name)) {
      std::ostringstream msg;
      msg << "Type " << callee_type->name << " does not have field '" << field_name << "'";

      Neeilang::error(expr->name, msg.str()); 
      expr_types[expr] = Primitives::TypeError();
      return;
    }

    // TODO : Check that this is an l-value.
    
    expr_types[expr] = callee_type;
}

void TypeChecker::visit(const Set * expr)
{
    auto expr_type = check(&expr->value);
    auto callee_type = check(&expr->callee);

    if (has_type_error( {expr_type, callee_type} )) {
      expr_types[expr] = Primitives::TypeError();
      return;
    }

    if (!expr_type->subclass_of(callee_type.get())) {
      Neeilang::error(expr->name, "Incompatible types in Set expression"); 
      expr_types[expr] = Primitives::TypeError();
      return;
    }

    expr_types[expr] = callee_type;
}

void TypeChecker::visit(const Grouping * expr)
{
   expr_types[expr] = check(&expr->expression);
}

void TypeChecker::visit(const Logical * expr)
{
    std::shared_ptr<Type> lhs_type = check(&expr->left);
    std::shared_ptr<Type> rhs_type = check(&expr->left);
    if (has_type_error( { lhs_type, rhs_type } )) {
      expr_types[expr] = Primitives::TypeError();
      return;
    }
    
    if (!match(lhs_type, { Primitives::Int(), Primitives::Float() })) {
      Neeilang::error(expr->op, "Left operand of logical operator must be Bool"); 
      expr_types[expr] = Primitives::TypeError();
      return;
    }

    if (!match(rhs_type, { Primitives::Int(), Primitives::Float() })) {
      Neeilang::error(expr->op, "Right operand of logical operator must be Bool"); 
      expr_types[expr] = Primitives::TypeError();
      return;
    }

    expr_types[expr] = Primitives::Bool();
}

void TypeChecker::visit(const Unary * expr)
{
    std::shared_ptr<Type> rhs_type = check(&expr->right);
    if (has_type_error( {rhs_type} )) {
      expr_types[expr] = Primitives::TypeError();
      return;
    }

    if ( !match(rhs_type, { Primitives::Int(), Primitives::Float() })) {
      Neeilang::error(expr->op,
                   "Right side of unary expression must be a number.");
      expr_types[expr] = Primitives::TypeError();
      return;
    }

    expr_types[expr] = rhs_type;
}

void TypeChecker::visit(const BoolLiteral *expr) {
  expr_types[expr] = Primitives::Bool();
}

void TypeChecker::visit(const NumLiteral * expr) {
  expr_types[expr] = Primitives::Int();
}

void TypeChecker::visit(const StrLiteral *expr) {
  expr_types[expr] = Primitives::String(); 
}

bool TypeChecker::match(const Expr * expr, const std::vector<std::shared_ptr<Type>> & types)
{
    std::shared_ptr<Type> expr_type = check(expr);
    if (has_type_error({expr_type})) return false;
    return match(expr_type, types);
}

bool TypeChecker::match(const std::shared_ptr<Type> expr_type, const std::vector<std::shared_ptr<Type>> & types) {
    for (auto type : types)
    {
        if (expr_type->subclass_of(type.get())) {
          return true;
        }
    }
    return false;
  
}

bool TypeChecker::has_type_error(const std::vector<std::shared_ptr<Type>> & types)
{
    for (auto type : types)
    {
        if (type == Primitives::TypeError()) {
          return true;
        }
    }
    return false;
}

