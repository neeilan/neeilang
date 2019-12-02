#include "reachability.h"
#include "cfg.h"
#include "neeilang.h"

#include <cassert>

#define ENSURE_REACHABLE(tok)                                                  \
  if (!graph.empty() && graph.back()->returns_at_node()) {                     \
    Neeilang::error(tok, "Unreachable statement");                             \
    return;                                                                    \
  }

#define ANALYZE_ONLY_WITHIN_FNS                                                \
  if (gather_fns_pass)                                                         \
    return;

using BasicBlock = NL::BasicBlock;
using Reachability = NL::Reachability;

void Reachability::analyze_program(const std::vector<Stmt *> &program) {
  // Reachability analysis is only performed within functions.
  gather_fns_pass = true;
  analyze(program);
  gather_fns_pass = false;
  for (const FuncStmt *fn : funcs) {
    visit(fn);
  }
}

void Reachability::analyze(const std::vector<Stmt *> &stmts) {
  for (const Stmt *stmt : stmts) {
    analyze(stmt);
  }
}

void Reachability::analyze(const Stmt *stmt) { stmt->accept(this); }

void Reachability::visit(const ClassStmt *cls) {
  in_class = true;
  analyze(cls->methods);
  in_class = false; // No nested classes right now.
}

void Reachability::visit(const FuncStmt *stmt) {
  if (gather_fns_pass) {
    funcs.push_back(stmt);
    return;
  }

  graph.clear();
  auto fn = BasicBlock::fn_entry(stmt->is_void());
  graph.push_back(fn);
  analyze(stmt->body);

  // init() doesn't have a return statement.
  bool is_init = in_class && stmt->name.lexeme == "init";
  if (is_init)
    return;

  if (!fn->is_void_func() && !fn->returns()) {
    Neeilang::error(stmt->name,
                    "Non-Void function has return-less code path(s).");
  }
}

void Reachability::visit(const BlockStmt *stmt) {
  analyze(stmt->block_contents);
}

void Reachability::visit(const ReturnStmt *stmt) {
  ANALYZE_ONLY_WITHIN_FNS
  ENSURE_REACHABLE(stmt->keyword)
  graph.back()->set_returns(true);
}

void Reachability::visit(const IfStmt *stmt) {
  ANALYZE_ONLY_WITHIN_FNS
  ENSURE_REACHABLE(stmt->keyword)

  assert(!graph.empty() && "Predecessor entry BB does not exist");

  std::shared_ptr<BasicBlock> pred = graph.back();
  /* Does this statement result in an unconditional return? */
  bool if_always_rets = false, else_always_rets = false;

  if (stmt->then_branch) {
    auto then_bb = BasicBlock::non_entry();
    pred->add_successor(then_bb);
    graph.push_back(then_bb);

    analyze(stmt->then_branch);
    if_always_rets = then_bb->returns();
  }

  if (stmt->else_branch) {
    auto else_bb = BasicBlock::non_entry();
    pred->add_successor(else_bb);
    graph.push_back(else_bb);

    analyze(stmt->else_branch);
    else_always_rets = graph.back()->returns();
  }

  if (if_always_rets && else_always_rets) {
    // This unconditional-return BB does not need to be a
    // successor of pred - it's only needed for detecting
    // any dead code that follows this if-else statement.
    graph.push_back(BasicBlock::unconditional_return());
  } else {
    // Insert a possibly-conditionally-returning post-loop BB
    auto post_if = BasicBlock::non_entry();
    pred->add_successor(post_if);
    graph.push_back(post_if);
  }
}

// These statements do not affect intra-function control flow; all
// we do is ensure they aren't dead code within a function.
void Reachability::visit(const ExprStmt *stmt) {
  ANALYZE_ONLY_WITHIN_FNS
  ENSURE_REACHABLE(stmt->sc)
}

void Reachability::visit(const PrintStmt *stmt) {
  ANALYZE_ONLY_WITHIN_FNS
  ENSURE_REACHABLE(stmt->keyword)
}

void Reachability::visit(const VarStmt *stmt) {
  ANALYZE_ONLY_WITHIN_FNS
  ENSURE_REACHABLE(stmt->name)
}

void Reachability::visit(const WhileStmt *stmt) {
  ANALYZE_ONLY_WITHIN_FNS
  ENSURE_REACHABLE(stmt->while_tok)
}

#undef ANALYZE_ONLY_WITHIN_FNS
#undef ENSURE_REACHABLE
