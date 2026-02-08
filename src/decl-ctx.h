#pragma once

#include <map>
#include <memory>
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
    if (!parent_) {
      return mappings.count(k) > 0;
    }
    return mappings.count(k) > 0 || parent_->contains(k);
  }

  V get(K const& k) {
    if (mappings.count(k) > 0 || !parent_) {
      return mappings[k].value;
    }
    return parent_->get(k);
  }

  bool contains(QualifiedName const& qn) const {
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

  V get(QualifiedName const& k) {
    return {};
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
};

using DeclCtx = DeclCtxImpl<std::string, Decl>;
