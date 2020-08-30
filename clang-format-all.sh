#!/bin/sh

# Do clang-format for all codes in LJF project

find . ! -path ./googletest/\* \
    -a \( -name \*.cpp -o -name \*.hpp -o -name \*.c -o -name \*.h \) \
    -exec /Volumes/Shared/clang+llvm-10.0.0-x86_64-apple-darwin/bin/clang-format -i {} +
