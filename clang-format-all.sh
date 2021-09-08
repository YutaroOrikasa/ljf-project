#!/bin/sh

# Do clang-format for all codes in LJF project

find . ! -path ./googletest/\* \
    -a \( -name \*.cpp -o -name \*.hpp -o -name \*.c -o -name \*.h \) \
    -exec clang-format -i {} +
