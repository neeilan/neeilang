#ifndef _NL_CACTUS_TABLE_H_
#define _NL_CACTUS_TABLE_H_

#include <map>
#include <memory>

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

  CactusTable() {}

  explicit CactusTable(std::string const& name)
  : CactusTable(nullptr, name) {}

  explicit CactusTable(ptr_t parent, std::string const& name = "")
      : parent_(parent), name_(name) {}

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

  std::string name() {
    if (!parent_) {
      return name_;
    }
    return parent_->name() + "::" + name_;
  }
};

#endif // _NL_CACTUS_TABLE_H_
