#ifndef _NL_BACKENDS_X86_64_CODEGEN_H_
#define _NL_BACKENDS_X86_64_CODEGEN_H_

#include <sstream>
#include <fstream>
#include <ostream>
#include <string>

#include "backends/x86-64/label.h"
#include "backends/x86-64/address.h"
#include "backends/x86-64/register.h"
#include "backends/x86-64/syscall.h"

namespace X86_64 {
class CodeGen {
public:
  CodeGen();
  void exit(const int status);
  void dump(std::ostream & out);
  Address str_const(const std::string & s);
  void print(const Address & addr);
  void begin_func(const std::string & name);
  const Register & add(const Register & a, const Register & b);
  const Register & move(const Register & dest, const Register & src);
  const Register load_const(int value);
  void call(const std::string & label);
  void ret();
  void print(const Register & r);

  void push(const Register & r);
  void pop(const Register & r);
  const Register pop();

private:
  std::ostringstream _asm;
  std::ostringstream _data;
  std::ostringstream _bss;
  std::ostringstream & _instr();
};
}

#endif // _NL_BACKENDS_X86_64_CODEGEN_H_
