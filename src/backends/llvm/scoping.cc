#include <memory>

#include "codegen.h"

void CodeGen::enter_scope() {
  sm.enter();
  named_vals = std::make_shared<NamedValueTable>(named_vals);
}

void CodeGen::exit_scope() {
  sm.exit();
  named_vals = named_vals->parent;
}
