#include <system_error>

#include "backends/llvm/codegen.h"

#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

/*
 * Dump a bitcode (bc) file if compilation succeeds
 *
 * This file can then be used to create an executable for the
 * target architecture as follows:
 *
 * 1) Use the LLVM static compiler (llc) to native assembly with `llc out.bc`
 * 2) Use the GNU toolchain for building an executable from the generated asm
 * file (out.s): gcc out.s -o executable
 *
 * Step 2 simply assembles the object file and links against host system
 * libraries to produce an executable.
 *
 * On Mac OS, this can be done "manually" in 2 steps:
 *
 * 1) as -o assembled out.s
 * 2) ld -macosx_version_min 10.11.0 -o executable assembled -lSystem
 */
void CodeGen::write_bitcode() {
  std::error_code EC;
  llvm::raw_fd_ostream os("out.bc", EC, llvm::sys::fs::F_None);
  llvm::WriteBitcodeToFile(*module, os);
  os.flush();
}
