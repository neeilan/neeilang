# /usr/bin/sh
bin/neeilang "$1" &> output.s
if [ $? -eq 0 ]; then
  gcc -o a.out output.s
  if [ $? -eq 0 ]; then
    ./a.out
  else
    echo "GCC: Assembler/Linker failure"
  fi
else
  cat output.s
  echo "NL: Compilation failure"
fi