# /usr/bin/sh

echo "Running splat tests..."
for test in test/functional/*.splat ; do
  echo $test
  bin/splat ./nl_x86.sh $test
  echo
done
