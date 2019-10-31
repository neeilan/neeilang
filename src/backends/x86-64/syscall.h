/*
Implements part of the syscall calling convention.
=================================================

Per https://www.uclibc.org/docs/psABI-x86_64.pdf :

Calling Conventions

The Linux AMD64 kernel uses internally the same calling conventions as user-
level applications (see section 3.2.3 for details). User-level applications that like
to call system calls should use the functions from the C library. The interface
between the C library and the Linux kernel is the same as for the user-level appli-
cations with the following differences:

1. User-level applications use as integer registers for passing the sequence
%rdi, %rsi, %rdx, %rcx, %r8 and %r9. The kernel interface uses %rdi,
%rsi, %rdx, %r10, %r8 and %r9.
2. A system-call is done via the syscall instruction. The kernel destroys
registers %rcx and %r11.
3. The number of the syscall has to be passed in register %rax.
4. System-calls are limited to six arguments, no argument is passed directly on
the stack.
5. Returning from the syscall, register %rax contains the result of the
system-call. A value in the range between -4095 and -1 indicates an error,
it is -errno.
6. Only values of class INTEGER or class MEMORY are passed to the kernel.
*/

#ifndef __NS_SYSCALL_H__
#define __NS_SYSCALL_H__

#if __APPLE__
  // From /usr/include/sys/syscall.h
  #define SYSCALL_write "0x2000004"
  #define SYSCALL_exit  "0x2000001"
#elif __linux__
#endif

#include <string>
#include <ostream>

using std::ostream;

#define _endl '\n'

template <typename T>
void syscall_id(ostream & out, T arg) {
  out << "mov rax, " << arg << _endl;
}

template <typename T>
void syscall_arg1(ostream & out, T arg) {
  out << "mov rdi, " << arg << _endl;
}

template <typename T>
void syscall_arg2(ostream & out, T arg) {
  out << "mov rsi, " << arg << _endl;
}

template <typename T>
void syscall_arg3(ostream & out, T arg) {
  out << "mov rdx, " << arg << _endl;
}

#endif // __NS_SYSCALL_H__

