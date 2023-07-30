# /usr/bin/sh
if bin/neeilang $1 &> output
then
  lli output
else
  echo "Compilation failed"
fi
