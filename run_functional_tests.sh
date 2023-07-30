# /usr/bin/sh

echo "Running splat tests..."
for test in test/functional/*.splat ; do
  bin/splat ./nl_llvm.sh $test
done
