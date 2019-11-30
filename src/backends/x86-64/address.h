#ifndef __NL_BACKENDS_X86_64_REGISTER_H__
#define __NL_BACKENDS_X86_64_REGISTER_H__

#include <string>

class Address {
public:
  Address(const std::string loc, int size)
    : loc(loc), size(size)  {}
  const std::string loc;
  int size; // in bytes
};

#endif //__NL_BACKENDS_X86_64_REGISTER_H__
