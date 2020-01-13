#ifndef _NL_BACKENDS_LLVM_OBJECT_H_
#define _NL_BACKENDS_LLVM_OBJECT_H_

/*
This file defines the memory layout and ABI for Neeilang objects in
the LLVM backend generated code. ABI compatibility with other backends
is planned, but not guaranteed at this time.

To prevent layout changes from breaking backend code, use the provided
object/field utility functions rather than using the LLVM API directly
and calculating field offsets manually.
*/

#include <vector>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"

extern int NL_OBJ_GC_BYTE_IDX;
extern int NL_OBJ_VT_IDX;

/* Returns the (ordered) object layout LLVM types. The header is:
 ___________________________________
| 1 byte for GC | 8 byte VTable ptr |
 ------------------------------------
This is expected to expand in the future, for example, for
synchronization (object-level locking).
*/
std::vector<llvm::Type *> object_header(llvm::LLVMContext &ctx);

/* Returns the object header size (# of fields, NOT bytes). */
int obj_header_size(llvm::LLVMContext &ctx);

#endif // _NL_BACKENDS_LLVM_OBJECT_H_
