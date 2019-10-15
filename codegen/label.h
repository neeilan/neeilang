#ifndef __NS_LABEL_H__
#define __NS_LABEL_H__

#include <cstdlib>
#include <string>
#include <sstream>

static std::size_t curr_labelnum;

std::string generate_label(const std::string & name) {
  std::ostringstream buf;
  buf << "label_" << curr_labelnum++ << "_" <<  name;
  return buf.str();
}

std::string generate_label() {
  return generate_label("");
}

#endif // __NS_LABEL_H__

