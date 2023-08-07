#ifndef _NL_BACKENDS_X86_64_STACKFRAME_H_
#define _NL_BACKENDS_X86_64_STACKFRAME_H_

#include <optional>
#include <string>
#include <unordered_map>

#include "primitives.h"
#include "scope-manager.h"
#include "stmt.h"
#include "visitor.h"

namespace x86_64 {

// 'Base' because other things like function args and spilled registers may
// get pushed onto the stack within the same function, but the local vars will
// always constitute the first elements of the stack throughout the lifetime
// of the function call.
struct FrameBase {
  void addLocal(const VarStmt *varStmt, uint16_t size) {
    sizes.push_back({varStmt, size});
    totalSize += size;
  }
  std::optional<uint16_t> bpOffsetOf(const VarStmt *varStmt) {
    uint16_t bpOffset = 0;
    for (auto &[v, size] : sizes) {
      if (v == varStmt) {
        return bpOffset;
      }
      bpOffset += size;
    }
    return std::nullopt;
  }
  std::vector<std::pair<const VarStmt *, uint16_t>> sizes = {{nullptr, 8}};
  uint16_t totalSize = 8;
};

class StackFrameSizer : public StmtVisitor<> {
 public:
  StackFrameSizer(ScopeManager &sm) : sm_(sm) {}
  void init(const std::vector<Stmt *> &program);

  OVERRIDE_STMT_VISITOR_FNS(void)

  std::unordered_map<const FuncStmt *, FrameBase> bases;

 private:
  void init(const Stmt *stmt);

  ScopeManager &sm_;
  const FuncStmt *enclosingFunc = nullptr;
};
}  // namespace x86_64

#endif  // _NL_BACKENDS_X86_64_STACKFRAME_H_
