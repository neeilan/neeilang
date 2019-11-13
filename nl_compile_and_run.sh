# /usr/bin/sh
if bin/neeilang $1 &> output
then
  ~/Desktop/llvm/build/bin/lli output
else
  echo "Compilation failed"
fi
