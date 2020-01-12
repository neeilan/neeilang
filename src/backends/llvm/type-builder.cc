#include <cassert>
#include <vector>

#include "type-builder.h"

#include "backends/llvm/object.h"
#include "primitives.h"

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"

llvm::Type *TypeBuilder::to_llvm(NLType t) {
  if (ll_types.find(t) != ll_types.end()) {
    return ll_types[t];
  }

  // We don't support function type fields/variables (yet?)
  assert(t->functype == NULL);

  std::vector<llvm::Type *> field_types;

  // Inheritance
  if (t->supertype) {
    llvm::Type *parent = to_llvm(t->supertype);
    for (llvm::Type *field :
         static_cast<llvm::StructType *>(parent)->elements()) {
      field_types.push_back(field);
    }
  }

  // Create the identified struct type
  auto opaque_struct = llvm::StructType::create(ctx, t->name);
  ll_types.insert({t, llvm::PointerType::getUnqual(opaque_struct)});

  for (auto hdr_field_ty : object_header(ctx)) {
    field_types.push_back(hdr_field_ty);
  }

  for (auto field : t->fields) {
    field_types.push_back(to_llvm(field.type));
  }

  opaque_struct->setBody(field_types);

  return ll_types[t];
}

llvm::FunctionType *TypeBuilder::to_llvm(std::shared_ptr<FuncType> f,
                                         NLType receiver) {
  llvm::Type *ret_type = to_llvm(f->return_type);
  std::vector<llvm::Type *> arg_types;

  // Methods take object pointer as first arg
  if (receiver) {
    arg_types.push_back(to_llvm(receiver));
  }

  for (NLType nl_argtype : f->arg_types) {
    arg_types.push_back(to_llvm(nl_argtype));
  }

  return llvm::FunctionType::get(ret_type, arg_types, false);
}
