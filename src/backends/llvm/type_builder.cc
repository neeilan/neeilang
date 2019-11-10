#include <cassert>
#include <vector>

#include "type_builder.h"

#include "primitives.h"

#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"

llvm::Type* TypeBuilder::to_llvm(NLType t)
{
    if (ll_types.find(t) != ll_types.end()) {
        return ll_types[t];
    }

    // We don't support function type fields/variables (yet?).
    assert(t->functype == NULL);

    std::vector<llvm::Type*> field_types;

    // Inheritance.
    if (t->supertype) {
        llvm::Type* parent = to_llvm(t->supertype);
        for (llvm::Type* field : static_cast<llvm::StructType*>(parent)->elements()) {
            field_types.push_back(field);
        }
    }

    // Create the identified struct type
    auto opaque_struct = llvm::StructType::create(ctx, t->name);
    ll_types.insert({ t,  llvm::PointerType::getUnqual(opaque_struct)  });

    for (auto field : t->fields) {
        field_types.push_back(to_llvm(field.type));
    }

    opaque_struct->setBody(field_types);

    return ll_types[t];
}
