#ifndef _NL_NEEILANG_H_
#define _NL_NEEILANG_H_

#include <string>

#include "token.h"

class Neeilang {
public:
  static void run_file(const char *path);

  static void error(std::vector<const char*> inclPath, int line, const std::string &message);

  static void error(Token token, const std::string &message);

private:
  static bool had_error;

  static void report(std::vector<const char*> inclPath, int line, std::string const& occurrence,
                    std::string const& message);
};

#endif // _NL_NEEILANG_H_