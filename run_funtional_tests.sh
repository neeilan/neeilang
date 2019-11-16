# /usr/bin/sh

echo "Running splat tests..."
for test in test/functional/*.splat ; do
  bin/splat ./neeilang_compile_and_run.sh test  
done
