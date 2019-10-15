#include <sstream>
#include <fstream>
#include <ostream>
#include <string>

#include "label.h"
#include "register.h"
#include "syscall.h"

#define EXPLAIN(exp) << " ; \t\t" << exp << endl


static const char endl = '\n';


class Address {
public:
  Address(const std::string loc, int size)
    : loc(loc), size(size)  {}
  const std::string loc;
  int size; // in bytes
};

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


CodeGen::CodeGen() {
  _asm << "default rel" << endl
       << "global start"  << endl
       << "section .text"  << endl
       << endl
       << "start:"        << endl;

  _data << "section .data" << endl
        << "newline db 10" << endl;

  _bss << "section .bss" << endl
       << "  dump_reg resb 8" << endl; // For debugging (print register contents).
}

void CodeGen::print(const Address & addr) {
  syscall_id(_instr(), SYSCALL_write);
  syscall_arg1(_instr(), "1"); // 1 here means STDOUT
  syscall_arg2(_instr(), addr.loc);
  syscall_arg3(_instr(), addr.size);
  _instr() << "syscall" << endl;
}

void CodeGen::print(const Register & r) {
  _instr() << endl;
  const Register temp = load_const(48);
  add(temp, r); // offset by 48 to get ASCII code for digits.
  _instr() << "mov [dump_reg], " << temp.name << endl;
  temp.free(); // We've written to memory, can re-use temp.

  print(Address("dump_reg", 8));
  print(Address("newline", 1));
}

void CodeGen::call(const std::string & label) {
  _instr() << "call " << label << endl;
}

void CodeGen::ret() {
  _instr() << "ret " << endl;
}

const Register CodeGen::load_const(int value) {
  const Register r = allocate_reg64();
  _instr() << "mov " << r.name << ", " << value << endl;
  return r;
}

void CodeGen::begin_func(const std::string & name) {
  const std::string label = generate_label(name);
  _asm << endl << label << ":" << endl;
}

const Register & CodeGen::add(const Register & a, const Register & b) {
  _instr() << "add " << a.name << ", " << b.name << endl;
  return a;
}

const Register & CodeGen::move(const Register & dest, const Register & src) {
  // MOV copies the contents of its source (second) operand into its destination (first) operand.
  _instr() << "mov " << dest.name << ", " << src.name << endl;
  return dest;
}

Address CodeGen::str_const(const std::string & s) {
  const std::string label = generate_label();
  _data << "    " << label << " db " << "\"" <<  s << "\"" << ", 10" EXPLAIN("10 is newline char");
  return Address(label, s.size() + 1); // +1 for newline
}

std::ostringstream & CodeGen::_instr() {
  _asm << "\t";
  return _asm;
}

void CodeGen::dump(std::ostream & out) {
  out << _data.str() << endl
      << _bss.str() << endl
      << _asm.str() << endl;
}

void CodeGen::exit(const int status) {
  syscall_id(_instr(), SYSCALL_exit);
  _instr() << "mov edi, " << status  EXPLAIN("edi holds the exit status");
  _instr() << "syscall" << endl;
}


void CodeGen::push(const Register & r) {
  _instr() << "push " << r.name  << endl;
}

void CodeGen::pop(const Register & r) {
  _instr() << "pop " << r.name  << endl;
}

const Register CodeGen::pop() {
  const Register r = allocate_reg64();
  pop(r);
  return r;
}

int main() {
  CodeGen cg;
  std::ofstream out("out.asm");

/*
  const Address label = cg.str_const("hello world");
  //cg.print(label);
  const Address label2 = cg.str_const("Bye world!");
  //cg.print(label2);

  cg.call("label_2_math"); // This should be populated later
*/

  Register arg = cg.load_const(4);
  cg.push(arg);
  arg.free();
  cg.call("label_1_print_digit");
  cg.exit(15);

  cg.begin_func("math");
  const Register a = cg.load_const(3);
  const Register b = cg.load_const(6);
  Register res1 = cg.add(a, b); // 9
  cg.print(res1);
  Register res2 = cg.add(a, a); // B (18)
  cg.print(res2);
  // No register spilling yet, so mark these as available for reuse.
  a.free(); b.free(); res1.free(); res2.free();
  cg.ret();

  cg.begin_func("print_digit");
  const Register return_addr = cg.pop();
  const Register num_to_print = cg.pop();
  cg.print(num_to_print);
  cg.push(return_addr);
  cg.ret();

  cg.dump(out);
  out.close();
  return 0;
}
