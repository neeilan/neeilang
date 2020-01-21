Installation & Usage
====================

Installation

Currently, NeeiLang can only be built from source. Prerequisites
to build the NL binary:
  
  - C++11 support (gcc ~4.7,  clang ~3.1)
  - CMake 2.8+
  - LLVM 4.0.0 (or compatible)

For obtaining and building LLVM sources, refer to [1] and [2].
After installation, clone the NL sources and build with CMake:

```
$ git clone https://github.com/neeilan/neeilang
$ cd neeilang/
$ # Replace the LLVM install path below
$ cmake CMakeLists.txt -DLLVM_PATH:STRING="~/Desktop/llvm"
$ make
$ bin/neeilang program.nl
```


Usage

Programs compiled with the NeeiLang compiler can be executed in two ways:

1) Directly via lli [3] : Successful compilation outputs LLVM
   assembly to stderr, which lli can execute:

   $ bin/neeilang source.nl &> output
   $ lli output

2) Via llc [4] : A bitcode file (out.bc) is also dumped when
   compilation succeeds. First, use the LLVM static compiler 
   to produce native assembly:

   $ llc out.bc   # produces out.s

   Then use your system's assembler and linker to produce an
   executable. The GNU toolchain can combine these steps:

   $ gcc out.s -o executable

   The above command is equivalent to assembling and linking 
   separately. For example, on macOS:

   $ as -o assembled out.s
   $ ld -macosx_version_min 10.11.0 -o executable assembled -lSystem
  

Resources

[1] https://llvm.org/docs/GettingStarted.html
[2] http://releases.llvm.org/
[3] https://llvm.org/docs/CommandGuide/lli.html
[4] https://llvm.org/docs/CommandGuide/llc.html
