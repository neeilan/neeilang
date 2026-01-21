#ifndef _NL_CONST_EVAL_H_
#define _NL_CONST_EVAL_H_

#include "ast-printer.h"
#include "expr.h"
#include "stmt.h"
#include "visitor.h"

#include <iostream>
#include <optional>
#include <unordered_map>

class FunctionTemplateArgSub : public ExprVisitor<const Expr*>,
                               public StmtVisitor<const Stmt*> {
public:
    void doSubstitution(const FuncStmt* f, TypeParse sub) {
        substitutions["T"] = sub;
        std::cout << "*** Substitution with T=" << sub.prettyName() << std::endl;
        std::cout << AstPrinter{}.print(substitute(f));
        std::cout << "\n*****************************" << std::endl;
        
    }

  std::unordered_map<std::string, TypeParse> substitutions;

  const Expr* substitute(const Expr *expr) { return expr->accept(this); }
  const Stmt* substitute(const Stmt *stmt) { return stmt->accept(this); }

  OVERRIDE_EXPR_VISITOR_FNS(const Expr*)
  OVERRIDE_STMT_VISITOR_FNS(const Stmt*)
};

const Stmt* FunctionTemplateArgSub::visit(const FuncStmt *stmt) {
    TypeParse ret = stmt->return_type;
    if (auto it = substitutions.find(ret.name.str()); it != substitutions.end()) {
        ret = it->second;
    }

    std::vector<TypeParse> parameter_types = stmt->parameter_types;
    for (auto& p : parameter_types) {
        if (auto it = substitutions.find( p.name.str()); it != substitutions.end()) {
             p = it->second;
        }
    }

    std::vector<const Stmt*> body;
    for (auto& s : stmt->body) {
        body.push_back(substitute(s));
    }


    return new FuncStmt(
        stmt->name,
        stmt->parameters,
        parameter_types,
        ret,
        body
    );
}

const Stmt* FunctionTemplateArgSub::visit(const NamespaceStmt *stmt) {
    assert(false); return nullptr;
}

const Stmt* FunctionTemplateArgSub::visit(const ClassStmt *stmt) {
    assert(false); return nullptr;
}

const Stmt* FunctionTemplateArgSub::visit(const ScopedEnum *stmt) {
    assert(false); return nullptr;
}
const Stmt* FunctionTemplateArgSub::visit(const TemplateStmt *stmt) {
    assert(false); return nullptr;
}

const Stmt* FunctionTemplateArgSub::visit(const UsingStmt *) {
    assert(false); return nullptr;
}

const Stmt* FunctionTemplateArgSub::visit(const AliasStmt *) {
    assert(false); return nullptr;
}


const Stmt* FunctionTemplateArgSub::visit(const BlockStmt * stmt) {
    std::vector<const Stmt *> new_contents;
    for (auto* s : stmt->block_contents) {
        new_contents.push_back(substitute(s));
    }
    return new BlockStmt(new_contents);
}

const Stmt* FunctionTemplateArgSub::visit(const ExprStmt *stmt) {
    return new ExprStmt(substitute(stmt->expression), stmt->sc);
}

const Stmt* FunctionTemplateArgSub::visit(const PrintStmt *stmt) {
  return new PrintStmt(stmt->keyword, substitute(stmt->expression));
}


const Stmt* FunctionTemplateArgSub::visit(const StaticAssertStmt *stmt) {
    return new StaticAssertStmt(substitute(stmt->value));
}

const Stmt* FunctionTemplateArgSub::visit(const VarStmt *stmt) {
    // TODO: See a type, so possible substitution here, but just do a deep-ish clone first
    // NOTE: A good litmus test would be having a variable like:
    // T::value_type x;
    auto tp = stmt->tp;
    if (auto it = substitutions.find(tp.name.str()); it != substitutions.end()) {
        tp = it->second;
    }
  return new VarStmt(stmt->name, tp, substitute(stmt->expression));
}

const Stmt* FunctionTemplateArgSub::visit(const IfStmt *stmt) {
  return new IfStmt(
    stmt->keyword,
    substitute(stmt->condition),
    substitute(stmt->then_branch),
    stmt->else_branch ? substitute(stmt->else_branch) : nullptr);
}

const Stmt* FunctionTemplateArgSub::visit(const WhileStmt *stmt) {
    return new WhileStmt(stmt->while_tok, substitute(stmt->condition), substitute(stmt->body));
}

const Stmt* FunctionTemplateArgSub::visit(const ReturnStmt *stmt) {
    return new ReturnStmt(stmt->keyword, substitute(stmt->value));
}

const Expr* FunctionTemplateArgSub::visit(const Binary *expr) {
    return new Binary(*substitute(&expr->left), expr->op, *substitute(&expr->right));
}

const Expr* FunctionTemplateArgSub::visit(const Get *expr) {
    return new Get(*substitute(&expr->callee), expr->name, expr->accessOp);
}

const Expr* FunctionTemplateArgSub::visit(const Set *expr) {
    return new Set(*substitute(&expr->callee), expr->name, *substitute(&expr->value));
}

const Expr* FunctionTemplateArgSub::visit(const SetIndex *expr) {
    return new SetIndex(*substitute(&expr->callee), expr->bracket, *substitute(&expr->index), *substitute(&expr->value));
}

const Expr* FunctionTemplateArgSub::visit(const GetIndex *expr) {
    return new GetIndex(*substitute(&expr->callee), expr->bracket, *substitute(&expr->index));
}

const Expr* FunctionTemplateArgSub::visit(const SizeOf *expr) {
    // TODO: Need to visit the variant here
    return expr;
}

const Expr* FunctionTemplateArgSub::visit(const AlignOf *expr) {
    // TODO: Need to propagate to type-id
    return expr;
}

const Expr* FunctionTemplateArgSub::visit(const Call *expr) {
    // TODO: args need substitution
    return new Call(*substitute(&expr->callee), expr->paren, {});
}

const Expr* FunctionTemplateArgSub::visit(const StrLiteral *expr) {
    return expr;
}

const Expr* FunctionTemplateArgSub::visit(const NumLiteral *expr) {
    return expr;
}

const Expr* FunctionTemplateArgSub::visit(const BoolLiteral *expr) {
    return expr;
}

const Expr* FunctionTemplateArgSub::visit(const Grouping *expr) {
    return new Grouping(*substitute(&expr->expression));
}

const Expr* FunctionTemplateArgSub::visit(const StaticCast *expr) {
    // TODO: Need to sub typeid to support e.g. CRTP
    return new StaticCast(expr->typeId, substitute(expr->expr));
}

const Expr* FunctionTemplateArgSub::visit(const Unary *expr) {
    return new Unary(expr->op, *substitute(&expr->right));
}

const Expr* FunctionTemplateArgSub::visit(const This *expr) {
    return new This(expr->keyword);
}

const Expr* FunctionTemplateArgSub::visit(const Variable *expr) {
    // TODO: Maybe need to visit qualified names too
    return new Variable(expr->name);
}

const Expr* FunctionTemplateArgSub::visit(const Assignment *expr) {
    // TODO: Maybe need to visit qualified names too
    return new Assignment(expr->name, *substitute(&expr->value));
}

const Expr* FunctionTemplateArgSub::visit(const Logical *expr) {
    return new Logical(*substitute(&expr->left), expr->op, *substitute(&expr->right));
}

const Expr* FunctionTemplateArgSub::visit(const SentinelExpr *expr) {
    assert(false); return nullptr;
}

#endif //_NL_CONST_EVAL_H_
