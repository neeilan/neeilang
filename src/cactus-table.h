#ifndef _NL_CACTUS_TABLE_H_
#define _NL_CACTUS_TABLE_H_

#include <map>
#include <memory>
#include "name.h"

/*
 * CactusTable implements a multi-map with scoping semantics.
 * It is a cactus tree (wikipedia.org/wiki/Parent_pointer_tree)
 * that is specializable on K+V to build symbol/type tables.
 */
template <typename K, typename V> class CactusTable {
private:
  std::map<K, V> mappings;

public:
  using ptr_t = std::shared_ptr<CactusTable<K, V>>;
  ptr_t parent_;
  std::string name_;
  V meta;
  int depth = 0;

  CactusTable() {}

  explicit CactusTable(std::string const& name)
  : name_(name) {}

  // TODO: Merge namespaces i.e. if parent already has this child
  explicit CactusTable(ptr_t parent, std::string const& name = "")
      : parent_(parent), name_(name), depth(parent_->depth+1) {}

  void insert(K const& k, V const& v) { mappings[k] = v; }

  bool contains(K const& k) const {
    if (!parent_) {
      return mappings.count(k) > 0;
    }
    return mappings.count(k) > 0 || parent_->contains(k);
  }

  V get(K const& k) {
    if (mappings.count(k) > 0 || !parent_) {
      return mappings[k];
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

#endif // _NL_CACTUS_TABLE_H_
