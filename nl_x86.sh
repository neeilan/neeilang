# /usr/bin/sh
bin/neeilang "$1" 1> output.s 2> nl_stderr.txt
if [ $? -eq 0 ]; then
  gcc -o a.out output.s
  if [ $? -eq 0 ]; then
    ./a.out
  else
    echo "GCC: Assembler/Linker failure"
  fi
else
  cat nl_stderr.txt
  echo "NL: Compilation failure"
fi
