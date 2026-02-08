#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "ast-printer.h"
#include "global-hoister.h"
#include "neeilang.h"
#include "parser.h"
#include "reachability.h"
#include "resolver.h"
#include "scanner.h"
#include "scope-manager.h"
#include "token.h"
#include "type-checker.h"

#include "backends/ir/irgen.h"

#ifdef TARGET_X86
#include "backends/x86-64/codegen.h"
#else
#include "backends/llvm/codegen.h"
#endif

bool Neeilang::had_error = false;

void Neeilang::run_file(const char *path) {
  Scanner scanner(path);
  const std::vector<Token> tokens = scanner.scan_tokens();

  // for (Token t : tokens) {
  //   std::cout << t.str() << std::endl;
  // }

  Parser parser(tokens);
  std::vector<const Stmt *> program = parser.parse();

  if (had_error) {
    return;
  }

  AstPrinter printer;
  std::cerr << printer.print(program);

  IRGen irgen;
  // irgen.generate(program);
  // irgen.dump();

//   Resolver resolver;
//   resolver.resolve_program(program);

//   if (had_error) {
//     return;
//   }

//   ScopeManager scope_manager;

//   GlobalHoister hoister(scope_manager);
//   hoister.hoist_program(program);

//   if (had_error) {
//     return;
//   }

//   NL::Reachability dce;
//   dce.analyze_program(program);

//   if (had_error) {
//     return;
//   }

//   TypeChecker type_checker(scope_manager);
//   type_checker.check(program);

//   if (had_error) {
//     return; // Compilation halted due to type errors.
//   }

// #ifdef TARGET_X86
//   x86_64::CodeGen codegen(type_checker.get_expr_types(), scope_manager);
//   codegen.generate(program);
//   codegen.dump();
// #else
//   CodeGen codegen(scope_manager, type_checker.get_expr_types());
//   codegen.generate(program);

//   if (!had_error)
//   {
//     // codegen.print();
//     codegen.write_bitcode();
//   }
// #endif
}

void Neeilang::error(std::vector<const char*> inclPath, int line, const std::string &message) {
  report(inclPath, line, "", message);
}

void Neeilang::error(Token token, const std::string &message) {
  if (token.type == END_OF_FILE) {
    report(token.inclPath, token.line, " at end", message);
  } else {
    report(token.inclPath, token.line, " at '" + token.lexeme + "'", message);
  }
}

// Private

void Neeilang::report(std::vector<const char*> inclPath, int line, std::string const& occurrence,
                      std::string const& message) {

  size_t depth = 0;
  for (size_t i = 0; i < inclPath.size() - 1; ++i) {
    std::cout << std::string(depth, ' ') << "[In file included from " << inclPath[i] << "]\n";
    depth++;
  }

  std::cout << std::string(depth, ' ') << "[" << inclPath.back() <<  ":" << line << "] Error: ";
  if (occurrence.size() > 0) {
    std::cout << occurrence << " : ";
  }
  std::cout << message << std::endl;

  had_error = true;
}
