#include "backends/x86-64/codegen.h"

#include <iostream>
#include <functional>

#include "arrays.h"
#include "primitives.h"

namespace x86_64 {

// TODO: Copied from LLVM backend - put in common utils
static bool is_initializer(const std::string &fn) {
  const std::string init("_init");
  if (fn.length() >= init.length()) {
    return (0 == fn.compare(fn.length() - init.length(), init.length(), init));
  } else {
    return false;
  }
}
// TODO: Copied from LLVM backend - put in common utils
/* Fetches the correct implementation of method that should be used
 for dynamic dispatch with type. Note that NL doesn't allow name reuse
 within a class, so the method name alone suffices here. However, it can
 be mangled to allow such behavior in the future. */
std::string get_virtual_method(NLType type, const std::string &method, const std::unordered_set<std::string> funcLabels) {
  NLType curr = type; // Curr is self or parent
  for (NLType curr = type; curr != nullptr; curr = curr->supertype) {
    auto methodName = curr->name + "_" + method;
    if (funcLabels.contains(methodName)) {
      return methodName;
    }
  }
  assert(false && "No such virtual fn");
}

ValueRefTracker::ValueRef CodeGen::emitArrayInit(NLType nlType,
                                const std::vector<const Expr *>& dims) {

  auto innerType = Arrays::next_enclosed_type(nlType);
  assert(innerType == Primitives::Int() && "Only Int arrays supported");
  assert(dims.size() == 1);
  // TODO: check that each dim is > 0.
  emit(dims[0]);
  text_.instr({"push", "%rdi"});
  text_.instr({"push", "%rsi"});
  // rdi holds # of elems
  text_.instr({"mov", valueRefs_.get(dims[0]), "%rdi"});
  // rdi now holds # of elems * size per elem
  text_.instr({"imul", "$8", "%rdi"});
  // rdi now holds another 8 bytes of header
  text_.instr({"add", "$8", "%rdi"});
  // Align the stack if necessary
  // TODO: Can we wrap this in an AlignedCall RAII?
  auto const stackLocals = stackFrames_.bases[enclosingFunc_];
  // == 0 because we push rsi - can we track this with RAII or similar?
  if (stackLocals.totalSize % 16 == 0) {
    text_.instr({"push", "%rbx"});
  }
  // %rdi holds array size
  text_.instr({"call", "malloc"});
  if (stackLocals.totalSize % 16 == 0) {
    text_.instr({"pop", "%rbx"});
  }

  text_.instr({"pop", "%rsi"});
  // Restore %rdi
  text_.instr({"pop", "%rdi"});
  // %rax is a pointer to the malloc'd memory
  // Array header { u32: size of each element, u32: number of elements }
  text_.instr({"movl", "$8", "(%rax)"});
  auto dimRef = valueRefs_.get(dims[0]);
  text_.instr({"movl", dimRef[0] == '%' ? (dimRef+"d") : dimRef , "4(%rax)"});
  return "%rax";
}

ValueRefTracker::ValueRef CodeGen::emitClassInit(NLType nlType) {
  // Total sizes of members
  // + vtable ptr
  std::function<uint32_t(NLType)> sizeOfMembers = [&](NLType t) -> uint32_t {
    if (!t) { return 0; }
    return t->fields.size() * 8 + sizeOfMembers(t->supertype);  
  };
  // Total size is sizeOfMembers + 8 bytes for a vtable pointer
  text_.instr({"push", "%rdi"});
  text_.instr({"push", "%rsi"});
  text_.instr({"mov", std::string("$")+std::to_string(sizeOfMembers(nlType)+8), "%rdi" });
  text_.instr({"call", "malloc"});
  text_.instr({"pop", "%rsi"});
  text_.instr({"pop", "%rdi"});

  // First 8 bytes are the vtable pointer
  text_.instr({"lea", "vtable_" + nlType->name + "(%rip)", "%r15"});
  text_.instr({"mov", "%r15", "(%rax)"});

  // %rax is a pointer to the malloc'd memory
  return "%rax";
}

void CodeGen::generate(const std::vector<Stmt *> &program) {
  sm_.reset();
  stackFrames_.init(program);
  sm_.reset();
  // Setup format strings for printf
  rodata_.directive({"format_printf_int: .asciz \"%ld\\n\""});
  rodata_.directive({"format_printf_float: .asciz \"%f\\n\""});
  text_.directive({".global main"});
  emit(program);

  for (auto *c : classes_) {
    auto className = c->name.lexeme;
    auto const classType = sm_.globals().typetab->get(className);
    rodata_.directive({ std::string("vtable_") + className + ":"});
    for (auto m : classType->get_methods()) {
      rodata_.directive({ std::string(".quad ") + get_virtual_method(classType, m->name, funcLabels_)});
    }
  }
}

void CodeGen::emit(const std::vector<Stmt *> &stmts) {
  for (const Stmt *stmt : stmts)
    emit(stmt);
}

void CodeGen::emit(const Stmt *stmt) { stmt->accept(this); }
void CodeGen::emit(const Expr *expr) { expr->accept(this); }

void CodeGen::visit(const ExprStmt *stmt) {
  emit(stmt->expression);
  // Clear all registers, unless we're using a preallocation scheme,
  // which we're current not.
  valueRefs_.resetRegisters();
}
void CodeGen::visit(const BlockStmt *stmt) {
  enterScope();
  emit(stmt->block_contents);
  exitScope();
}

void CodeGen::visit(const PrintStmt *stmt) {
  AstPrinter ap;
  text_.instr({"# BEGIN print", ap.print(stmt->expression)});
  auto const *e = stmt->expression;
  if (!e) {
    return;
  }
  emit(e);
  auto const exprRef = valueRefs_.get(e);

  // Align the stack if necessary
  auto const stackLocals = stackFrames_.bases[enclosingFunc_];
  if (stackLocals.totalSize % 16) {
    text_.instr({"push", "%rbx"});
  }

  auto const exprType = exprTypes_.find(e);
  if (exprType->second == Primitives::String()) {
    text_.instr({"push", "%rdi"});
    if (stackLocals.totalSize % 16) { text_.instr({"push", "%rbx"}); }
    text_.instr({"mov", exprRef, "%rdi"});
    text_.instr({"call", "puts"});
    // This sucks - the library should hide this stack-aligning stuff
    if (stackLocals.totalSize % 16) { text_.instr({"pop", "%rbx"}); }
    text_.instr({"pop", "%rdi"});
  } else if (exprType->second == Primitives::Float()) {
    text_.instr({"lea", "format_printf_float(%rip)", "%rdi"});
    // TODO: Assumes the float is a literal, which isn't always true
    text_.instr({"movsd", exprRef + "(%rip)", "%xmm0"});
    text_.instr({"call", "printf"});
  } else {
    // %rdi and %rsi hold first two integer/pointer function params
    // per x86-64 System V calling convention
    // Preserve/restore %rdi and %rsi *carefully*

    // This is hacky - we want to use r14/r15 as GP, not just
    // compiler scratch.
    text_.instr({"push", "%rdi"});
    text_.instr({"push", "%rsi"});
    text_.instr({"lea", "format_printf_int(%rip)", "%rdi"});
    text_.instr({"mov", exprRef, "%rsi"});
    text_.instr({"call", "printf"});
    text_.instr({"pop", "%rsi"});
    text_.instr({"pop", "%rdi"});
  }
  if (stackLocals.totalSize % 16) {
    text_.instr({"pop", "%rbx"});
  }
  valueRefs_.regFree(exprRef);
  text_.instr({"# END print", ap.print(stmt->expression)});
}

void CodeGen::visit(const VarStmt *stmt) {
  auto const &varName = stmt->name.lexeme;
  auto const nlType = sm_.current().typetab->get(varName);
  if (stmt->expression) {
    emit(stmt->expression);
  } else {
    if (nlType->is_array_type()) {
    valueRefs_.assign(stmt->expression, emitArrayInit(nlType, stmt->tp.dims));
    } else {
      // TODO: Pick reasonable defaults, based on type.
      valueRefs_.assign(stmt->expression, "$0");
    }
  }


  // For globals, space should be reserved in .data and addressed relative to %rip
  auto const bpOffset = stackFrames_.bases[enclosingFunc_].bpOffsetOf(stmt);
  assert(bpOffset.has_value());

  auto val = valueRefs_.get(stmt->expression);
  valueRefs_.regFree(valueRefs_.get(stmt->expression));

  // dest is the memory location of this variable on the stack
  // to start off with, assume only one int64 variable, first thing on the stack
  auto const dest = (*bpOffset ? (std::string("-") + std::to_string(*bpOffset)) : std::string{}) + "(%rbp)";
  // 8 bytes so use q suffix
  bool mustRestoreVal = false;
  if (val[0] != '%' && dest[0] != '%' && val[0] != '$') {
    auto [valReg, mustRestore] = valueRefs_.acquireRegister(stmt->expression);
    text_.instr({"mov", val, valReg});
    val = valReg; mustRestoreVal = mustRestore;
  }
  text_.instr({ "movq", val, dest});
  if (mustRestoreVal) {
    text_.instr({"pop", val});
  }
  namedVals->insert(varName, dest);
}

void CodeGen::visit(const ClassStmt *stmt) {
  classes_.push_back(stmt);
  enclosingClass_ = sm_.current().typetab->get(stmt->name.lexeme);;
  enterScope();
  for (const Stmt *method : stmt->methods) {
    emit(method);
  }
  exitScope();
  enclosingClass_ = nullptr;
}

void CodeGen::visit(const IfStmt *stmt) {
  static uint16_t id = 1;
  emit(stmt->condition);
  const auto ref = valueRefs_.get(stmt->condition);
  // TODO: Is is fine if cond is memory and not register?
  auto const cond = valueRefs_.makeAssignable(stmt->condition);
  if (ref != cond) {
    text_.instr({"mov", ref, cond });
    valueRefs_.regFree(ref); // TODO: Can we do this by reference-counting?
  }

  auto elseLabel = std::string("__else_") + std::to_string(id++);
  auto postIfStmtLabel = std::string("__post_ifstmt_") + std::to_string(id);
  // cond AND cond
  // cond & cond - set flags
  // if cond is true - ZF = 0 ; jne jumps
  // else            = ZF = 1 ;  je jumps
  text_.instr({"test", cond, cond });
  // je = jump if ZF=1 (i.e cond is 0)
  text_.instr({"je", stmt->else_branch ? elseLabel : postIfStmtLabel});
  valueRefs_.regFree(cond);
  emit(stmt->then_branch);
  if (stmt->else_branch) {
    text_.instr({"jmp", postIfStmtLabel});
    text_.label({elseLabel});
    emit(stmt->else_branch);
  }
  text_.label({postIfStmtLabel});
}

void CodeGen::visit(const WhileStmt *stmt) {
  static uint16_t id = 1;
  auto const checkCondLabel =
      std::string("__loop_check_") + std::to_string(id);
  auto const postLoopLabel = std::string("__post_loop_") + std::to_string(id++);

  text_.label({checkCondLabel});
  emit(stmt->condition);
  auto cond = valueRefs_.get(stmt->condition);
  bool mustRestoreCond = false;
  if (cond[0] != '%') {
    auto const [condReg, mustRestore] =
        valueRefs_.acquireRegister(stmt->condition);
    text_.instr({"mov", cond, condReg});
    cond = condReg;
    mustRestoreCond = mustRestore;
  }
  text_.instr({"test", cond, cond});
  text_.instr({"je", postLoopLabel});
  valueRefs_.regFree(cond);
  if (stmt->body) {
    emit(stmt->body);
  }
  text_.instr({"jmp", checkCondLabel});
  text_.label({postLoopLabel});
  if (mustRestoreCond) {
    text_.instr({"pop", cond});
  }
}

void CodeGen::visit(const FuncStmt *stmt) {
  enterScope();
  auto *oldenclosingFunc_ = enclosingFunc_;
  enclosingFunc_ = stmt;
  auto label = [&]() {
    std::string name;
    if (enclosingClass_) {
      name += enclosingClass_->name;
      name += "_";
    }
    return name + stmt->name.lexeme;
  }();
  funcLabels_.insert(label);
  text_.label({label});
  text_.instr({"pushq", "%rbp"});

  // In x86 and x86-64 assembly, the stack pointer %rsp points to the next empty
  // slot on the stack, not to a valid value. It always points to the memory
  // location that will be used for the next push operation.
  // Also, values 'grow' towards lower addresses.
  auto const stackLocalsBase = stackFrames_.bases[stmt];
  text_.instr({"movq", "%rsp", "%rbp"});
  text_.instr({"subq", "$" + std::to_string(stackLocalsBase.totalSize),
               "%rsp"});  // locals sit between bp and sp

  emit(stmt->body);
  
  // Void functions may not have return stmt
  // TODO: Insert this in reachability stage!
  if (text_.contents.back().values[0] != "ret") {
    ReturnStmt tmp({}, nullptr);
    emit(&tmp);
  }

  enclosingFunc_ = oldenclosingFunc_;
  exitScope();
}

void CodeGen::visit(const ReturnStmt *stmt) {
  if (stmt->value) {
    emit(stmt->value);
    text_.instr({"mov", valueRefs_.get(stmt->value), "%rax"});
  }

  text_.instr({"mov", "%rbp", "%rsp"});
  text_.instr({"popq", "%rbp"});
  text_.instr({"ret"});
}

void CodeGen::visit(const Unary *expr) {
  auto const exprType = exprTypes_.find(&expr->right);
  if (exprType->second == Primitives::Float()) {
    // Unimplemented
    return;
  }
  emit(&expr->right);
  auto const r = valueRefs_.get(&expr->right);
  auto const dest = valueRefs_.makeAssignable(&expr->right);
  if (r != dest) {
    text_.instr({"mov", r, dest });
  }

  if (expr->op.type == TokenType::MINUS) {
    text_.instr({"neg", dest});
  } else { // BANG
    text_.instr({"not", dest});
  }
  valueRefs_.assign(expr, dest);
}

void CodeGen::visit(const Binary *expr) {
  // TODO(neeilan): Explore passing left register to accumulate
  emit(&expr->left);
  auto const left = valueRefs_.get(&expr->left);
  auto const dest = valueRefs_.makeAssignable(&expr->left);
  // We can't add into a literal, so we need an 'assignable' dest
  // Also, if (call) + (call) don't want to clobber %rax
  if (left != dest) {
    valueRefs_.regOverwrite(&expr->left, dest);
    text_.instr({"mov", left, dest });
  }

  emit(&expr->right);
  auto const right = valueRefs_.get(&expr->right);
  valueRefs_.regFree(left);

  auto binaryOpEmit = [&](auto const &opcode){
    // We use `right` as the first operand because
    // sub behaves as dest-=right
    text_.instr({ opcode, right, dest });
    valueRefs_.regOverwrite(expr, dest);
    // Can reuse `right` as dest is the accumulator
    valueRefs_.regFree(right);
  };

  auto cmpOpEmit = [&](auto const &setByteOp){
    auto destByte = dest[0] == '%' ? (dest+"b") : dest;
    text_.instr({"cmp", right, dest });
    text_.instr({setByteOp, destByte});
    valueRefs_.regOverwrite(expr, dest);
    valueRefs_.regFree(right);
  };

  switch (expr->op.type) {
  // For the following instructions, we know both operands are Ints or Floats
  // (since we don't handle String concat with '+' yet).
  case PLUS: {
    binaryOpEmit("add");
    break;
  }
  case MINUS: {
    binaryOpEmit("sub");
    break;
  }
  case STAR: {
    binaryOpEmit("imul");
    break;
  }
  case SLASH: {
    auto destl = (dest[0] == '%' ? dest+"d" : dest);
    text_.instr({"xor", "%rax", "%rax" });
    text_.instr({"movl", destl, "%eax" });
    text_.instr({"movl", right, "%ecx" });
    text_.instr({"cltd" }); // sign-extends eax into edx:eax
    text_.instr({"idivq", "%rcx" });
    text_.instr({"movq", "%rax", dest });
    valueRefs_.regOverwrite(expr, dest);
    valueRefs_.regFree(right);
    break;
  }
  case GREATER: {
    cmpOpEmit("setg");
    break;
  }
  case LESS: {
    cmpOpEmit("setc");
    break;
  }
  case GREATER_EQUAL: {
    cmpOpEmit("setge");
    break;
  }
  case LESS_EQUAL: {
    cmpOpEmit("setle");
    break;
  }
  case EQUAL_EQUAL: {
    cmpOpEmit("sete");
    break;
  }
  case BANG_EQUAL: {
    cmpOpEmit("setne");
    break;
  }
  default: { std::cerr << "[Unimplemented BinaryOp]" << std::endl; }
  }
}

void CodeGen::visit(const Grouping *expr) {
  auto const *e = &expr->expression;
  emit(e);
  valueRefs_.overwrite(expr, valueRefs_.get(e));
}

void CodeGen::visit(const StrLiteral *expr) {
  static uint16_t strLiteralId = 1;
  // Reuse literals if possible
  static std::unordered_map<std::string, std::string> literalToLabel;
  auto it = literalToLabel.find(expr->value);
  if (it != literalToLabel.end()) {
    valueRefs_.assign(expr, it->second);
    return;
  }
  auto const label =
      std::string("__strlit_") + std::to_string(strLiteralId++);
  rodata_.directive({label + ": .asciz \"" + expr->value + "\""});
  literalToLabel[expr->value] = label;
  // Need memory references to be rip-relative to produce position independent
  // executables i.e we want the assembler to emit a RIP-relative relocation
  // rather than an absolute R_X86_64_32, since gcc invokes the linker in PIE
  // mode by default. Same for printf format strings below.
  // e.g. `lea __strlit_1(%rip), %rsi`
  auto const dest = valueRefs_.makeAssignable(expr);
  text_.instr({"lea", label+"(%rip)", dest});
  valueRefs_.assign(expr, dest);
}

void CodeGen::visit(const NumLiteral *expr) {
  auto const exprType = exprTypes_.find(expr);
  if(exprType == exprTypes_.end()) { std::cerr << "[Unknown ExprType]" << std::endl; return; }
  // TODO: How do negative literals work here?
  if (exprType->second == Primitives::Float()) {
    static uint16_t id = 1;
    auto const label = std::string("_float_literal_") + std::to_string(id++);
    rodata_.directive({label + ": .double " + expr->value});
    valueRefs_.assign(expr, label);
  } else if (exprType->second == Primitives::Int()) {
    // e.g. 5 becomes $5
    valueRefs_.assign(expr, "$" + expr->value);
  }
  
}

void CodeGen::visit(const BoolLiteral *expr) {
  const auto immediate = expr->value ? "$1" : "$0";
  valueRefs_.assign(expr, immediate);
}

void CodeGen::visit(const Variable *expr) {
  auto const &varName = expr->name.lexeme;
  auto key = TypeTableUtil::fn_key(varName);
  auto const nlType = sm_.current().typetab->get(key);
  // Referring to a function?
  if (nlType && nlType->is_function_type()) {
    valueRefs_.assign(expr, varName);
    return;
  }

  // Referring to a function parameter?
  auto const& params = enclosingFunc_->parameters;
  auto hasImplicitThisArg = enclosingClass_ != nullptr;
  static std::vector<std::string> argRegs = { "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9" };
  for (size_t i = 0; i < params.size(); ++i) {
    if (varName == params[i].lexeme) {
      valueRefs_.assign(expr, argRegs[i + hasImplicitThisArg]);
      return;
    }
  }

  valueRefs_.assign(expr, namedVals->get(varName));

}
void CodeGen::visit(const Assignment *expr) {
  emit(&expr->value);
  // Move into reg because x86 doesn't support memory-to-memory `mov`s
  auto const [srcReg, mustRestore] = valueRefs_.acquireRegister(&expr->value);
  text_.instr({"mov", valueRefs_.get(&expr->value), srcReg});
  auto const dest = namedVals->get(expr->name.lexeme);
  text_.instr({"mov", srcReg, dest});
  if (mustRestore) {
    text_.instr({"pop", srcReg});
  } else {
    valueRefs_.regFree(srcReg);
  }
  valueRefs_.assign(expr, dest);
}

void CodeGen::visit(const Logical *expr) {
  // TODO(neeilan): For short-circuiting, wait to emit right
  emit(&expr->left);
  emit(&expr->right);

  auto const left = valueRefs_.get(&expr->left);
  auto const right = valueRefs_.get(&expr->right);
  auto const dest = valueRefs_.makeAssignable(&expr->left);

  auto logicalOpEmit = [&](auto const &opcode) {
    text_.instr({opcode, right, dest});
    valueRefs_.regOverwrite(expr, dest);
    // Can resuse `right` as dest is the accumulator
    valueRefs_.regFree(right);
  };

  switch (expr->op.type) {
    case AND: {
    logicalOpEmit("and");
    return;
    }
    case OR: {
    logicalOpEmit("or");
    return;
    }
    default: {
    return;
    }
  }
}

void CodeGen::visit(const Call *expr) {
  // 3 cases:
  // 1) Free function [implemented below]
  // 2) Method
  // 3) Constructor
   emit(&expr->callee);
   auto callee = valueRefs_.get(&expr->callee);

  // Constructors
  // ------------
  auto const isInitializer = is_initializer(callee);
  auto const isMethodCall = isInitializer || callee[0] == '%';
  // If it's a constructor, we must first allocate the object.
  if (isInitializer) {
    auto const className = callee.substr(0, callee.find('_'));
    auto const classType = sm_.current().typetab->get(className);
    emitClassInit(classType);
    // Preserve the allocated address (rax) and rdi
    text_.instr({"push", "%rdi"});
    // Pass `this` as first arg
    text_.instr({"mov", "%rax", "%rdi"});
  } else if (isMethodCall) {
    // Pass `this` as first arg
    text_.instr({"push", "%rdi"});
    text_.instr({"mov", lastDereferencedObj_, "%rdi"});
  }

  // Per System V ABI
  static std::vector<std::string> argRegs = { "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9" };
  auto const numArgs = expr->args.size() + isMethodCall; // +1 for `this`
  assert(numArgs <= argRegs.size() && "Not enough registers to pass args");

  // Align the stack if necessary
  auto const stackLocals = stackFrames_.bases[enclosingFunc_];
  // We push/pop %rdi for method-like calls
  if (stackLocals.totalSize % (16 - 8*isMethodCall)) {
    text_.instr({"push", "%rbx"});
  }

  // Save scratch registers
  uint16_t numPushed = 0;
  for (size_t i = isMethodCall; i < numArgs; ++i) {
    text_.instr({"push", argRegs[i]});
    numPushed++;
  }
  if (numPushed % 2 == 1) {
    text_.instr({"push", "%rbx"});
  }

  for (size_t i = 0; i < expr->args.size(); i++) {
    const Expr *arg = expr->args[i];
    emit(arg);
    auto const rArg = valueRefs_.get(arg);
    // If method call, first register is used for `this`
    auto const targetReg = i + isMethodCall;
    text_.instr({"mov", rArg, argRegs[targetReg]});
    if (rArg != argRegs[targetReg]) {
      valueRefs_.regFree(rArg);
    }
  }

  // Preserve GP regs
  // TODO: No need if we could somehow tell that a function's transitive graph won't
  // use a specific register.
  std::vector<std::string> gpRegs{ "%r10", "%r11" , "%r12", "%r13"};
  for (auto & r : gpRegs) {
    text_.instr({"push", r});
  }

  text_.instr({"call", (callee[0] == '%' ? "*" : "") + callee});
  valueRefs_.regFree(callee);
  valueRefs_.assign(expr, "%rax");

  for (auto it = gpRegs.rbegin(); it != gpRegs.rend(); ++it) {
    text_.instr({"pop", *it});
  }

  // Restore scratch registers
  if (numPushed % 2 == 1) {
    text_.instr({"pop", "%rbx"});
  }
  for (size_t i = isMethodCall; i < numArgs; ++i) {
    text_.instr({"pop", argRegs[i]});
  }

  if (stackLocals.totalSize % (16 - 8*isMethodCall)) {
    text_.instr({"pop", "%rbx"});
  }

  if (isMethodCall) {
    text_.instr({"pop", "%rdi"});
  }
}
void CodeGen::visit(const Get *expr) {
  auto fieldName = expr->name.lexeme;
  auto calleeType = exprTypes_.find(&expr->callee);
  assert(calleeType != exprTypes_.end() && "NL Type of callee unknown");

  // Initializers / static fields
  if (calleeType->second == Primitives::Class()) {
    const Variable *callee = static_cast<const Variable *>(&expr->callee);
    const auto& className = callee->name.lexeme;
    if (fieldName == "init") {
      valueRefs_.assign(expr, className + "_init");
    }
    return;
  }

  // Emit the callee
  emit(&expr->callee);
  lastDereferencedObj_ = valueRefs_.get(&expr->callee);

  // Methods
  if (calleeType->second->has_method(fieldName)) {
    text_.instr({"# BEGIN method lookup: " + fieldName});  
    auto [reg, mustRestore] = valueRefs_.acquireRegister(expr);
    text_.instr({"mov", lastDereferencedObj_, reg});

    // reg holds address of the callee
    // therefore (reg) holds address of the vtable
    text_.instr({"mov", "(" + reg + ")", reg});
    // now reg holds address of the vtable
    // therefore reg[i] holds address of method i for this class
    auto idx = calleeType->second->method_idx(fieldName);
    if (auto offset = idx * 8) {
      text_.instr({"add", std::string("$") + std::to_string(offset), reg});
    }
    // Set reg to the address of the method
    text_.instr({"mov", "(" + reg + ")", reg});
    valueRefs_.assign(expr, reg);
    if (mustRestore) {
      text_.instr({"pop", reg});
    }
    text_.instr({"# END method lookup: " + fieldName});
    valueRefs_.regFree(lastDereferencedObj_);
    return;
  }

  // Plain fields
  auto idx = calleeType->second->field_idx(fieldName);
  // Value is idx * 8 byte offset into the address of the last deref object
  valueRefs_.assign(&expr->callee, lastDereferencedObj_);
  text_.instr({"movq", lastDereferencedObj_, "%rax"});

  // constexpr uint64_t fieldSize = 8;
  for (int i = 0; i <= idx; i++) {
    text_.instr({"add", "$8", "%rax"});
  }

  auto const fieldAccess = "(%rax)";
  valueRefs_.regFree(lastDereferencedObj_);
  auto res = valueRefs_.makeAssignable(expr);
  text_.instr({"movq",  fieldAccess, res});
  valueRefs_.assign(expr, res);
}

void CodeGen::visit(const Set *expr) {
  auto fieldName = expr->name.lexeme;
  auto calleeType = exprTypes_.find(&expr->callee);
  assert(calleeType != exprTypes_.end() && "NL Type of callee unknown");

  // Emit the callee
  emit(&expr->callee);
  auto const calleeRef = valueRefs_.get(&expr->callee);

  // Emit the value
  emit(&expr->value);
  auto valueRef = valueRefs_.get(&expr->value);

  auto idx = calleeType->second->field_idx(fieldName);
  // Value is idx * 8 byte offset into the address of the last deref object
  text_.instr({"movq", calleeRef, "%rax"});
  for (int i = 0; i <= idx; i++) {
    text_.instr({"add", "$8", "%rax"});
  }
  auto const fieldAccess = "(%rax)";

  bool mustRestoreVal = false;
  if (valueRef[0] != '%' && valueRef[0] != '$') {
    auto [valReg, mustRestore] = valueRefs_.acquireRegister(&expr->value);
    text_.instr({"mov", valueRef, valReg});
    valueRef = valReg; mustRestoreVal = mustRestore;
  }

  text_.instr({"movq", valueRef, fieldAccess});
  if (mustRestoreVal) { text_.instr({"pop", valueRef}); }

  valueRefs_.regFree(calleeRef);
  valueRefs_.regFree(valueRef);
}

void CodeGen::visit(const GetIndex * expr) {
  emit(&expr->callee);
  emit(&expr->index);

  // arr is array base - 8
  auto const arr = valueRefs_.makeAssignable(&expr->callee);
  text_.instr({"mov", valueRefs_.get(&expr->callee), arr});
  auto const index = valueRefs_.makeAssignable(&expr->index);
  text_.instr({"mov", valueRefs_.get(&expr->index), index});

  auto const elem = std::string("8(") + arr + ", " + index + ", 8)";
  valueRefs_.regFree(arr);

  valueRefs_.regFree(index);
  auto res = valueRefs_.makeAssignable(expr);
  valueRefs_.regFree(arr);

  text_.instr({"movq",  elem, res});
  valueRefs_.assign(expr, res);
}

void CodeGen::visit(const SetIndex *expr) {
  emit(&expr->callee);
  emit(&expr->index);
  emit(&expr->value);

  // arr is array base - 8
  auto const arr = valueRefs_.makeAssignable(&expr->callee);
  text_.instr({"mov", valueRefs_.get(&expr->callee), arr});
  auto const index = valueRefs_.makeAssignable(&expr->index);
  text_.instr({"mov", valueRefs_.get(&expr->index), index});

  auto const elem = std::string("8(") + arr + ", " + index + ", 8)";
  text_.instr({"movq", valueRefs_.get(&expr->value) , elem});
  valueRefs_.assign(expr, elem);
}

void CodeGen::visit(const This *expr) {
  // For a method call, `this` is the first argument passed in %rdi
  valueRefs_.assign(expr, "%rdi");
}

void CodeGen::visit(const SentinelExpr *) {}

void CodeGen::dump() const {
  std::stringstream ss;
  auto dumpLine = [&](auto const &asmLine) {
    auto const &line = asmLine.values;
    if (asmLine.isLabel()) {
      ss << line[0] << ":\n";
      return;
    }
    ss << ' ' << line[0];
    for (size_t i = 1; i < line.size() - 1; ++i) {
      ss << ' ' << line[i] << ", ";
    }
    if (line.size() > 1) {
      ss << ' ' << line.back();
    }
    ss << '\n';
  };
  auto dumpSection = [&](auto const &name, auto const &s) {
    ss << name << '\n';
    for (auto const &l : s.contents) {
      dumpLine(l);
    }
  };
  dumpSection(".section .rodata", rodata_);
  dumpSection(".data", data_);
  dumpSection(".text", text_);
  std::cout << ss.str();
}

} // namespace x86_64
