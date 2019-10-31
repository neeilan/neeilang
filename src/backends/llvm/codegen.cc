#include "expr.h"
#include "backends/llvm/codegen.h"

#include "llvm/IR/Value.h"

using llvm::Value;

void CodeGen::visit(const Unary *expr) {}
void CodeGen::visit(const Binary *expr) {}
void CodeGen::visit(const Grouping *expr) {}
void CodeGen::visit(const StrLiteral *expr) {}
void CodeGen::visit(const NumLiteral *expr) {}
void CodeGen::visit(const BoolLiteral *expr) {}
void CodeGen::visit(const Variable *expr) {}
void CodeGen::visit(const Assignment *expr) {}
void CodeGen::visit(const Logical *expr) {}
void CodeGen::visit(const Call *expr) {}
void CodeGen::visit(const Get *expr) {}
void CodeGen::visit(const Set *expr) {}
void CodeGen::visit(const This *expr) {}

