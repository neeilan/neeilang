/*
 * Implements bindings and helpers for interacting with host OS libc.
 */

#ifndef _NL_BACKENDS_LLVM_LIBC_H_
#define _NL_BACKENDS_LLVM_LIBC_H_

#include <cassert>
#include <string>

#include "primitives.h"
#include "type.h"

namespace NL_LLVM_libc {

// These format strings are specifically for NL 'print' statements.
const std::string printf_fmt_Int = "%d\n";
const std::string printf_fmt_Float = "%f\n";
const std::string printf_fmt_String = "%s\n";

const std::string print_fmtstr(NLType t) {
  if (t == Primitives::Int())
    return printf_fmt_Int;
  if (t == Primitives::Bool())
    return printf_fmt_Int;
  if (t == Primitives::Float())
    return printf_fmt_Float;
  if (t == Primitives::String())
    return printf_fmt_String;

  assert(false &&
         "No format str for type; type checker should have caught this");
  return ""; // Unreachable
}

} // namespace NL_LLVM_libc

#endif // _NL_BACKENDS_LLVM_LIBC_H_
