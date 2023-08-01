# /usr/bin/sh
bin/neeilang "$1" &> output.ll
if [ $? -eq 0 ]; then
  lli out.bc
else
  cat output.ll
  echo "NL: Compilation failure"
fi