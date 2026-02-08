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
    std::unordered_map<std::string, const Stmt*> cache_;

    void doSubstitution(const TemplateStmt* t, std::vector<TypeParse*> subs) {
        std::string key = std::to_string((uint64_t)t);
        for (auto const& s : subs) { key += s->prettyName(); }
        if (cache_.contains(key)) {
            std::cout << "Previously substituted\n";
            return;
        }


        assert(subs.size() <= t->args.size());
        templateInstDesc = "<";
        for (size_t i = 0; i < subs.size(); ++i) {
            substitutions[t->args[i].name.lexeme] = *subs[i];
            templateInstDesc += subs[i]->prettyName();
            if (i != subs.size() - 1) { templateInstDesc += ","; }
        }
        templateInstDesc += ">";
        assert(substitutions.size() == t->args.size());
        
        std::cout << " *** Template substitution for " << templateInstDesc << " ***\n";
        auto* res = substitute(t->decl);
        std::cout << AstPrinter{}.print(res);
        std::cout << "\n*****************************" << std::endl;

        cache_[key] = res;

        templateInstDesc.clear();
        substitutions.clear();
    }

protected:
    TypeParse substitute(TypeParse) const;
    QualifiedName substitute(QualifiedName const&) const;

    std::string templateInstDesc;

  std::unordered_map<std::string, TypeParse> substitutions;

  const Expr* substitute(const Expr *expr) {
    if (!expr) { return nullptr; }
    return expr->accept(this);
  }
  const Stmt* substitute(const Stmt *stmt) {
    if (!stmt) { return nullptr; }
    return stmt->accept(this);
  }

  OVERRIDE_EXPR_VISITOR_FNS(const Expr*)
  OVERRIDE_STMT_VISITOR_FNS(const Stmt*)
};

TypeParse FunctionTemplateArgSub::substitute(TypeParse tp) const {
    if (auto it = substitutions.find(tp.name.str()); it != substitutions.end()) {
        return it->second;
    }

    tp.name = substitute(tp.name);
    
    return tp;
}

QualifiedName FunctionTemplateArgSub::substitute(QualifiedName const& name) const {
    QualifiedName qn = name; qn.tokens.clear(); qn.tmplInstantiations.clear();

    // e.g. T::key_type (vs )
    // NOTE: ::T... - no substitution on the first parameter
    if (auto it = substitutions.find(name.tokens.front().lexeme);
        !name.isFullyQualified && it != substitutions.end()) {
        qn = it->second.name;
    } else {
        qn.tokens.push_back(name.tokens.front());
        auto const& ti = name.tmplInstantiations.front();
        if (!ti) {
            qn.tmplInstantiations.push_back(ti);
        } else {
            std::vector<TypeParse*> newInstantiation;
            for (auto const* tp : *ti) {
                newInstantiation.push_back(new TypeParse(substitute(*tp)));
            }
            qn.tmplInstantiations.push_back(newInstantiation);
        }
    }

    for (size_t i = 1; i < name.tokens.size(); ++i) {
        qn.tokens.push_back(name.tokens[i]);
        auto const& ti = name.tmplInstantiations[i];
        if (!ti) {
            qn.tmplInstantiations.push_back(ti);
        } else {
            std::vector<TypeParse*> newInstantiation;
            for (auto const* tp : *ti) {
                newInstantiation.push_back(new TypeParse(substitute(*tp)));
            }
            qn.tmplInstantiations.push_back(newInstantiation);
        }
    }

    return qn;
}

const Stmt* FunctionTemplateArgSub::visit(const FuncStmt *stmt) {
    std::vector<TypeParse> parameter_types = stmt->parameter_types;
    for (auto& p : parameter_types) {
        p = substitute(p);
    }

    std::vector<const Stmt*> body;
    for (auto& s : stmt->body) {
        body.push_back(substitute(s));
    }

    return new FuncStmt(
        stmt->name,
        stmt->parameters,
        parameter_types,
        substitute(stmt->return_type),
        body,
        stmt->ctx
    );
}

const Stmt* FunctionTemplateArgSub::visit(const NamespaceStmt *stmt) {
    assert(false); return nullptr;
}

const Stmt* FunctionTemplateArgSub::visit(const ClassStmt *stmt) {
    std::optional<TypeParse> superclass;
    if (superclass) {
        superclass = substitute(*superclass);
    }

    std::vector<TypeParse> fieldTypes;
    for (auto& ft : stmt->field_types) {
        fieldTypes.push_back(substitute(ft));
    }

    std::vector<const Stmt *> memberDecls;
    for (auto* s : stmt->memberDecls) {
        memberDecls.push_back(substitute(s));
    }

    return new ClassStmt(
        stmt->name,
        superclass,
        stmt->fields,
        fieldTypes,
        memberDecls,
        stmt->ctx
    );
}

const Stmt* FunctionTemplateArgSub::visit(const ScopedEnum *stmt) {

    std::vector<NamedEnumerator> enumerators = stmt->enumerators;
//   for (auto& e : enumerators) {
//  // TODO: value can be initiatized as a constexpr involving a templated type
//  // e.g. Foo<T>::intVal
//   }
    std::optional<TypeParse> underlying = stmt->underlying;
    if (underlying) {
    underlying = substitute(*underlying);
    }
    return new ScopedEnum(stmt->name, enumerators, underlying, stmt->ctx);
}
const Stmt* FunctionTemplateArgSub::visit(const TemplateStmt *stmt) {
    assert(false); return nullptr;
}

const Stmt* FunctionTemplateArgSub::visit(const UsingStmt *) {
    assert(false); return nullptr;
}

const Stmt* FunctionTemplateArgSub::visit(const AliasStmt * stmt) {
    return new AliasStmt(stmt->alias, substitute(stmt->origId));
}


const Stmt* FunctionTemplateArgSub::visit(const BlockStmt * stmt) {
    std::vector<const Stmt *> new_contents;
    for (auto* s : stmt->block_contents) {
        new_contents.push_back(substitute(s));
    }
    return new BlockStmt(new_contents, stmt->ctx);
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
  return new VarStmt(stmt->name, substitute(stmt->tp), substitute(stmt->expression));
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
