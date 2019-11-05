#include <cassert>
#include <vector>

#include "type_builder.h"

#include "primitives.h"

#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"

llvm::Type *TypeBuilder::to_llvm(NLType t) {
  if (ll_types.find(t) != ll_types.end()) {
    return ll_types[t];
  }

  // We don't support function type fields/variables (yet?).
  assert(t->functype == NULL);

  std::vector<llvm::Type *> field_types;

  // TODO: This doesn't handle non-primitive field types well.
  // Non-primitive field types in classes should really be
  // treated as pointers to that type.
  // i.e - class X { child : X } should internally be class X { child : X* };
  for (auto field : t->fields) {
    field_types.push_back(to_llvm(field.type));
  }

  ll_types.insert({t, llvm::StructType::create(ctx, field_types, t->name)});
  return ll_types[t];
}
