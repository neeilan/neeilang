#include "backends/x86-64/label.h"

#include <cstdlib>
#include <sstream>

static std::size_t curr_labelnum;

std::string generate_label(const std::string &name) {
  std::ostringstream buf;
  buf << "label_" << curr_labelnum++ << "_" << name;
  return buf.str();
}

std::string generate_label() { return generate_label(""); }
