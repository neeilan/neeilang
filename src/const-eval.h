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
    // TODO: This should be cached directly in the template's context
    // or even in the class/function/var stmt to prevent declcontext
    // clashes. Alternatively, keys here can use fully-qualified names.
    std::unordered_map<std::string, const Stmt*> cache_;

    void summarizeSubstitutions() const {
        if (cache_.empty()) {
            return;
        }
        std::cout << "Instantiated templates:\n";
        for (auto const& [k,_] : cache_) {
            std::cout << "\t" << k << "\n";
        }
    }

    const Stmt* maybeSubstitute(const Stmt* s) {
        return substitute(s);
    }

    void doSubstitution(const TemplateStmt* t, std::vector<TypeParse*> subs) {
        std::string key = std::to_string((uint64_t)t);
        // for (auto const& s : subs) { key += s->prettyName(); }
        // if (cache_.contains(key)) {
        //     std::cout << "Previously substituted\n";
        //     return;
        // }


        assert(subs.size() <= t->args.size());
        templateInstDesc = "<";
        for (size_t i = 0; i < subs.size(); ++i) {
            substitutions[t->args[i].name.lexeme] = *subs[i];
            templateInstDesc += subs[i]->prettyName();
            if (i != subs.size() - 1) { templateInstDesc += ","; }
        }
        templateInstDesc += ">";
        assert(substitutions.size() == t->args.size());
        
        // std::cout << " *** Template substitution for " << templateInstDesc << " ***\n";
        /*auto* res =*/ substitute(t->decl);
        // std::cout << AstPrinter{}.print(res);
        // std::cout << "\n*****************************" << std::endl;

        // cache_[key] = res;

        templateInstDesc.clear();
        substitutions.clear();
    }


    const Stmt* doSubstitution2(const Stmt* t, std::unordered_map<std::string, TypeParse> const& subs) {
        auto oldSubs = substitutions;
        for (auto const & [k, v] : subs) {
            substitutions[k] = v;
        }
        // std::cout << " *** Template substitution ***\n";
        auto* res = substitute(t);
        // std::cout << AstPrinter{}.print(res);
        // std::cout << "\n*****************************" << std::endl;
        substitutions = oldSubs;
        return res;
    }

protected:
    TypeParse substitute(TypeParse, DeclCtx::ptr_t ctx);
    QualifiedName substitute(QualifiedName const&, DeclCtx::ptr_t ctx);

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

  // to prevent infinite loops
  // e.g. template <typename T> struct foo { foo<T> makeCopy(); };
  // here we put foo<T> in the activeSubstitutions set, so when
  // foo<T> is encountered in makeCopy's return type, we can simply
  // remember to come back and store a pointer there.
  std::unordered_set<std::string> activeSubstitutions; 

  OVERRIDE_EXPR_VISITOR_FNS(const Expr*)
  OVERRIDE_STMT_VISITOR_FNS(const Stmt*)
};

TypeParse FunctionTemplateArgSub::substitute(TypeParse tp, DeclCtx::ptr_t ctx) {
    // std::cout << "trying to sub " << tp.prettyName() << "\n";
    if (auto it = substitutions.find(tp.name.str()); it != substitutions.end()) {
        return it->second;
    }

    tp.name = substitute(tp.name, ctx);
    
    return tp;
}

QualifiedName FunctionTemplateArgSub::substitute(QualifiedName const& name, DeclCtx::ptr_t ctx) {
    // A QN is simply a list of names, each of which are possibly templates.

    QualifiedName qn = name;

    for (size_t i = 0; i < name.tokens.size(); ++i) {
        bool isTempl = name.tmplInstantiations[i].has_value();
        if (isTempl) {
            // std::cout << "NOTE: " << name.tokens[i].lexeme << " in " << name.str() << " is a template, so instantiating it : \n";
            assert(ctx);
            if (!ctx->contains(name)) {
                throw ParseErr(name.tokens[i].lexeme + " not defined");
            }
            assert(ctx->contains(name));
            auto mapping = ctx->get(name);
            if (!mapping.isTemplate || !mapping.isType) {
                throw ParseErr(name.tokens[i].lexeme + " was expected to be a class template, but it's a " + mapping.str());
            }
            auto* klass = static_cast<const ClassStmt*>(mapping.stmt);
            assert(klass->tmplParams);

            auto requiredArgCount = klass->tmplParams->size();
            auto argCount = (*name.tmplInstantiations[i]).size();
            if (argCount != requiredArgCount) {
                throw ParseErr(name.tokens[i].lineDiagnostic() + name.tokens[i].lexeme
                    + " requires " + std::to_string(requiredArgCount) + " args to initialize but got " + std::to_string(argCount)); 
            }
            std::unordered_map<std::string, TypeParse> subs;
            for (size_t j = 0; j < argCount; ++j) {
                subs[(*klass->tmplParams)[j].name.lexeme] = *(*name.tmplInstantiations[i])[j];
            }

            // The post-substituted version of the qualified name refers to a concrete class
            // (the one we just instantiated from a class template)
            auto const canonicalTmplName = klass->canonicalClassTemplateName();
            std::string tmplInstArgs;
            for (auto const * arg : *name.tmplInstantiations[i]) {
                tmplInstArgs += substitute(*arg, ctx).prettyName() + ",";
            }
            if (!tmplInstArgs.empty()) {
                tmplInstArgs.pop_back();
            }
            auto const canonicalClassName = canonicalTmplName + "@" + tmplInstArgs;

            qn.tokens[i].lexeme = canonicalClassName; // A real class, no longer a template!
            qn.tmplInstantiations[i] = std::nullopt;



            if (activeSubstitutions.find( canonicalTmplName ) != activeSubstitutions.end()) {
                // Prevent infinite substitution loop - i.e. we already see e.g. foo<T> here.
                // Ofc, we need to handle cases where foo<T1> is defined in terms of foo<T2> but that's a
                // problem for later.
                continue;
            }
            
            if (cache_.find(canonicalClassName) == cache_.end()) {
                activeSubstitutions.insert(canonicalTmplName);
                cache_[canonicalClassName] = doSubstitution2(klass, subs); // TODO: cache this value in klass
                activeSubstitutions.erase(canonicalTmplName);
            }
        }
    }


    // QualifiedName qn = name; qn.tokens.clear(); qn.tmplInstantiations.clear();

    // // e.g. T::key_type (vs )
    // // NOTE: ::T... - no substitution on the first parameter
    // if (auto it = substitutions.find(name.tokens.front().lexeme);
    //     !name.isFullyQualified && it != substitutions.end()) {
    //     qn = it->second.name;
    // } else {
    //     qn.tokens.push_back(name.tokens.front());
    //     auto const& ti = name.tmplInstantiations.front();
    //     if (!ti) {
    //         qn.tmplInstantiations.push_back(ti);
    //     } else {
    //         std::vector<TypeParse*> newInstantiation;
    //         for (auto const* tp : *ti) {
    //             newInstantiation.push_back(new TypeParse(substitute(*tp, ctx)));
    //         }
    //         qn.tmplInstantiations.push_back(newInstantiation);
    //     }
    // }

    // for (size_t i = 1; i < name.tokens.size(); ++i) {
    //     qn.tokens.push_back(name.tokens[i]);
    //     auto const& ti = name.tmplInstantiations[i];
    //     if (!ti) {
    //         qn.tmplInstantiations.push_back(ti);
    //     } else {
    //         std::vector<TypeParse*> newInstantiation;
    //         for (auto const* tp : *ti) {
    //             newInstantiation.push_back(new TypeParse(substitute(*tp, ctx)));
    //         }
    //         qn.tmplInstantiations.push_back(newInstantiation);
    //     }
    // }

    return qn;
}

const Stmt* FunctionTemplateArgSub::visit(const FuncStmt *stmt) {
    std::vector<TypeParse> parameter_types = stmt->parameter_types;
    for (auto& p : parameter_types) {
        p = substitute(p, stmt->ctx);
    }

    std::vector<const Stmt*> body;
    for (auto& s : stmt->body) {
        body.push_back(substitute(s));
    }

    return new FuncStmt(
        stmt->name,
        stmt->parameters,
        parameter_types,
        substitute(stmt->return_type, stmt->ctx),
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
        superclass = substitute(*superclass, stmt->ctx);
    }

    std::vector<TypeParse> fieldTypes;
    for (auto& ft : stmt->field_types) {
        fieldTypes.push_back(substitute(ft, stmt->ctx));
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
    underlying = substitute(*underlying, stmt->ctx);
    }
    return new ScopedEnum(stmt->name, enumerators, underlying, stmt->ctx);
}
const Stmt* FunctionTemplateArgSub::visit(const TemplateStmt *stmt) {
    return stmt;
}

const Stmt* FunctionTemplateArgSub::visit(const UsingStmt *) {
    assert(false); return nullptr;
}

const Stmt* FunctionTemplateArgSub::visit(const AliasStmt * stmt) {
    return new AliasStmt(stmt->alias, substitute(stmt->origId, stmt->ctx));
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
  return new VarStmt(stmt->name, substitute(stmt->tp, stmt->ctx), substitute(stmt->expression));
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

const Stmt* FunctionTemplateArgSub::visit(const ExplicitClassTemplateInitialization *stmt) {
    // TODO: This should return a new class stmt
    auto const& templateClassName = stmt->name;
    if (templateClassName.tokens.size() != 1) {
        throw ParseErr("ExplicitClassTemplateInitialization only supported with single-token template (possibly fully-qualified)");
    }

    if (!stmt->ctx->contains(templateClassName)) {
        throw ParseErr("ExplicitClassTemplateInitialization - unrecognized template class name " + templateClassName.tokens[0].lexeme);
    }

    auto mapping = stmt->ctx->get(templateClassName); // mapping to the class stmt
    if (!mapping.isTemplate) {
        throw ParseErr("ExplicitClassTemplateInitialization - " + templateClassName.tokens[0].lexeme + " is not a class template");
    }
    auto* klass = static_cast<const ClassStmt*>(mapping.stmt);
    assert(klass->tmplParams);

    auto requiredArgCount = klass->tmplParams->size();
    auto argCount = templateClassName.tmplInstantiations.empty() || !templateClassName.tmplInstantiations[0] ? 0 : templateClassName.tmplInstantiations[0]->size();
    if (argCount != requiredArgCount) {
        throw ParseErr("ExplicitClassTemplateInitialization - " + templateClassName.tokens[0].lexeme + " requires "
            + std::to_string(requiredArgCount) + " args to initialize but got " + std::to_string(argCount));
    }
    std::cout << "requiredArgCount = " << requiredArgCount << " argCount =" << argCount << "\n";

    auto canonicalTmplName = klass->canonicalClassTemplateName();
    std::unordered_map<std::string, TypeParse> subs;
    std::string tmplInstArgs;
    for (size_t i = 0; i < argCount; ++i) {
        subs[(*klass->tmplParams)[i].name.lexeme] = *(*templateClassName.tmplInstantiations[0])[i];
        tmplInstArgs += substitute(*(*templateClassName.tmplInstantiations[0])[i], stmt->ctx).prettyName() + ",";
    }
    if (!tmplInstArgs.empty()) {
        tmplInstArgs.pop_back();
    }
    auto const canonicalClassName = canonicalTmplName + "@" + tmplInstArgs;

    if (cache_.find(canonicalClassName) == cache_.end()) {
        activeSubstitutions.insert(canonicalTmplName);
        cache_[canonicalClassName] = doSubstitution2(klass, subs); // TODO: cache this value in klass
        activeSubstitutions.erase(canonicalTmplName);
    }
    return cache_[canonicalClassName];
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
