# /usr/bin/sh
bin/neeilang "$1" &> output.ll
if [ $? -eq 0 ]; then
  lli output.ll
else
  cat output.ll
  echo "NL: Compilation failure"
fi