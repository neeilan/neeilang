#include <iostream>

#include "neeilang.h"
#include "token.h"

int main(int argc, char **argv) {
  if (argc > 2) {
    std::cout << "Usage: neeilang [source file]" << std::endl;
    exit(0);
  }

  Neeilang nl;

  if (argc == 2) {
    nl.run_file(argv[1]);
  }

  return 0;
}