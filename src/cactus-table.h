#ifndef _NL_CACTUS_TABLE_H_
#define _NL_CACTUS_TABLE_H_

#include <map>
#include <memory>

/*
 * CactusTable implements a multi-map with scoping semantics.
 * It is a cactus tree (wikipedia.org/wiki/Parent_pointer_tree)
 * that is specializable on K+V to build symbol/type/vtables.
 */
template <typename K, typename V> class CactusTable {
private:
  std::map<K, V> mappings;

public:
  std::shared_ptr<CactusTable<K, V>> parent;

  CactusTable() {}
  explicit CactusTable(std::shared_ptr<CactusTable<K, V>> parent)
      : parent(parent) {}

  void insert(K const& k, V const& v) { mappings[k] = v; }

  bool contains(K const& k) const {
    if (!parent) {
      return mappings.count(k) > 0;
    }
    return mappings.count(k) > 0 || parent->contains(k);
  }

  V get(K const& k) {
    if (mappings.count(k) > 0 || !parent) {
      return mappings[k];
    }
    return parent->get(k);
  }
};

#endif // _NL_CACTUS_TABLE_H_
