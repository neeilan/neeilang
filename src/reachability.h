#ifndef _NL_REACHABILITY_ANALYZER_H_
#define _NL_REACHABILITY_ANALYZER_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "cfg.h"
#include "stmt.h"
#include "visitor.h"

namespace NL {

class Reachability : public StmtVisitor<void> {
private:
  CFG graph;
  bool in_class = false;
  bool gather_fns_pass = false;
  std::vector<const FuncStmt *> funcs;

  void analyze(const Stmt *stmt);
  void analyze(const std::vector<Stmt *> &stmts);

public:
  void analyze_program(const std::vector<Stmt *> &program);
  STMT_VISITOR_METHODS(void)
};

} // namespace NL

#endif //_NL_REACHABILITY_ANALYZER_H_
