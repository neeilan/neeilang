#pragma once

#include <map>
#include <memory>
#include <cassert>
#include <iostream>
#include "name.h"

/*

__global ___________________________________
   |      |       |         |        |     |
ns foo   fn bar  type baz  var qux  tmpl  tmpl
 | | |   *vars*   |  |  |             |    |  
a  b c          var fn type          fn    type

*/
template <typename K, typename V> class DeclCtxImpl {
public:
  using ptr_t = std::shared_ptr<DeclCtxImpl<K, V>>;
  ptr_t parent_;
  std::string name_;
  int depth = 0;
  V value;
  std::map<K, ptr_t> mappings;

  DeclCtxImpl() {}

  explicit DeclCtxImpl(std::string const& name) : name_(name) {}

  // TODO: Merge namespaces i.e. if parent already has this child
  explicit DeclCtxImpl(ptr_t parent, std::string const& name, V value)
      : parent_(parent), name_(name), depth(parent_->depth+1), value(value) {}

  ptr_t& insert(ptr_t me, K const& k, V const& v) {
    auto & res = mappings[k];
    res = std::make_shared<DeclCtxImpl>(me, k, v);
    return res;
  }

  bool contains(K const& k) const {
    std::cout << "    ------- declctx [" << name_ << "] contents:\n";
    for (auto const& e : mappings) {
      std::cout << e.first  << e.second->value.str() <<  " ";
    }
    std::cout << "\n -------\n";

    if (!parent_) {
      // std::cout << "   no parent, checking if contains " << k << "\n";
      return mappings.count(k) > 0;
    }
    return mappings.count(k) > 0 || parent_->contains(k);
  }

  V get(K const& k) {
    if (mappings.count(k) > 0 || !parent_) {
      if (!parent_) {
      }
      return mappings[k]->value;
    }
    return parent_->get(k);
  }

  bool contains(QualifiedName const& qn, bool forceDown=false) const {
    // assert(qn.tokens.size() == 1);
    if (qn.tokens.size() == 1) {
      return contains(qn.tokens.front().lexeme);
    } else if (auto rt = qn.tokens.front().lexeme; contains(rt)) {
      std::cout << "Looking for first part of qn " <<rt << " in " << name_ << "\n";
      QualifiedName qn2 = qn;
      qn2.tokens.erase(qn2.tokens.begin());
      qn2.tmplInstantiations.erase(qn2.tmplInstantiations.begin());
      std::cout << " remaining qn is " << qn2.str() << std::endl;
      if (mappings.count(rt) > 0) {
        return mappings.at(rt)->contains(qn2, true);
      } else if (!forceDown) {
        return parent_->contains(qn2);
      } else {
        return false;
      }
    } else {
      return false;
    }
    // TODO: Account for tmplInstantiations
    /*
    e.g 
        ::foo::bar  - not handled here
        foo::bar    - 3 cases
                      1) Am *I* foo ? Look for bar (https://godbolt.org/z/TPcr8fbME)
                      2) Do I have foo namespace/class ? Look for bar in the child, strictly searching down
                      3) Does my parent know foo::bar ?
    */
    return false;
  }

  V get(QualifiedName const& qn) {
    if (qn.tokens.size() == 1) {
      return get(qn.tokens.front().lexeme);
    } else if (auto rt = qn.tokens.front().lexeme; contains(rt)) {
      QualifiedName qn2 = qn;
      qn2.tokens.erase(qn2.tokens.begin());
      qn2.tmplInstantiations.erase(qn2.tmplInstantiations.begin());
      if (mappings.count(rt) > 0) {
        return mappings.at(rt)->get(qn2);
      } else {
        return parent_->get(qn2);
      }

    } else {
      assert(false); // Always call contains() first
      return {};
    }
  }


  std::string name() {
    if (!parent_) {
      return name_;
    }
    return parent_->name() + "::" + name_;
  }
};

struct Stmt;

struct Decl {
    Stmt const* stmt;
    uint16_t isNamespace : 1;
    uint16_t isTemplate  : 1;
    uint16_t isType      : 1;
    uint16_t isFunc      : 1;
    uint16_t isVariable  : 1;
    uint16_t isBlock     : 1;
    uint16_t pad         : 10;

    std::string str() {
      std::string res = "|";
      if (isNamespace) res += "Ns|";
      if (isTemplate) res += "Tm|";
      if (isType) res += "Ty|";
      if (isFunc) res += "Fn|";
      if (isVariable) res += "Vr|";
      if (isBlock) res += "Bl|";
      return res;
    }
};

using DeclCtx = DeclCtxImpl<std::string, Decl>;
