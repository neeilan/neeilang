#ifndef _NL_NEEILANG_H_
#define _NL_NEEILANG_H_

#include <string>

#include "token.h"

class Neeilang {
public:
  static void run_file(const char *path);

  static void run(const std::string &source);

  static void error(int line, const std::string &message);

  static void error(Token token, const std::string &message);

private:
  static bool had_error;

  static void report(int line, const std::string &occurrence,
                     const std::string &message);
};

#endif // _NL_NEEILANG_H_