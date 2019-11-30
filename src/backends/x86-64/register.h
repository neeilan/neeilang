#ifndef __NS_REGISTER_H__
#define __NS_REGISTER_H__

#include <set>
#include <string>

class Register {
public:
  Register(const std::string name) : name(name) {};
  void free() const;
  std::string name;
};

Register allocate_reg64(); 

#endif // __NS_REGISTER_H__
