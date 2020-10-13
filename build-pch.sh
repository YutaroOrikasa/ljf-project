#!/bin/sh

set -ve

mkdir -p build/ljf-python/grammar
/Volumes/Shared/llvm-8/llvm-workspace/usr/bin/clang++ -Wfatal-errors -mlinker-version=519 -Wall -std=c++17 -MMD -MP -I./include -I./googletest/googletest/include -I./\
                -fno-exceptions ljf-python/grammar/expr.hpp -xc++-header -o build/ljf-python/grammar/expr.hpp.pch --relocatable-pch # -isysroot ./build

rm -rf ljf-python/grammar/expr.hpp.pch

mkdir -p ljf-python/grammar
/Volumes/Shared/llvm-8/llvm-workspace/usr/bin/clang++ -O2 -Wfatal-errors -mlinker-version=519 -Wall -std=c++17 -MMD -MP -I./include -I./googletest/googletest/include -I./\
                -fno-exceptions ljf-python/grammar/expr.hpp -xc++-header -o ljf-python/grammar/expr.hpp.pch

mkdir -p build/ljf-python
time /Volumes/Shared/llvm-8/llvm-workspace/usr/bin/clang++ -v -O2 -Wfatal-errors -mlinker-version=519 -Wall -std=c++17 -MMD -MP -I./include -I./googletest/googletest/include -I./ -I./build -include ljf-python/grammar/expr.hpp -c -fno-exceptions ljf-python/rppl.cpp -o build/ljf-python/rppl.cpp.o
