$ g++ -c src/backends/x86-64/runtime.cc
./nl_x86.sh memory_leak.nl
valgrind --tool=massif ./a.out