Implementation Notes
=====================

Frontend

The frontend includes the lexer, parser, and semantic analysis passes for
type-checking, variable resolution, dead code detection etc. It is written in
C++, takes in source code, and builds a NL AST and auxiliary data structures 
for compiler backends to utilize.

Backends    
 
LLVM: Emits LLVM bitcode, which can then retargeted for x86-64 on
Windows/Linux/macOS, ARM, PowerPC etc.

x86-64: Hand-written x86-64 backend that targets Linux and Mac OS. This
backend is still in a very early stage, but the plan is to emit x64 'stack
machine' assembly, then implement passes to correctly and efficiently use the
instruction set, native calling convention, and available registers. 
  
Runtime

NL is designed not to have an extensive runtime system. One of the few key
run-time services required by the 'spec' is automatic memory management.
