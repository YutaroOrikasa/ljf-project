# Please copy this file to config.mk like
# 		cp cconfig-template-linuxbrew.mk config.mk
# , uncomment lines and overwrite configuation.

# BUILD_DIR = 



# compiler options about llvm library.
# eg. LIBLLVM_CXXFLAGS = -I/usr/local/opt/llvm/include

LIBLLVM_CXXFLAGS = -I/home/linuxbrew/.linuxbrew/opt/llvm/include



# linker options about llvm library.
# eg. LIBLLVM_LDFLAGS = -L/usr/local/opt/llvm/lib

LIBLLVM_LDFLAGS = -L/home/linuxbrew/.linuxbrew/opt/llvm/lib -Wl,--rpath=/home/linuxbrew/.linuxbrew/opt/llvm/lib



# compiler command

CC = clang
CXX = clang++

# compiler/linker options

CFLAGS = -fPIC
CXXFLAGS = -fPIC
# LDFLAGS =



# ljf config file that is c++ header file.
# It may be copied from ljf-config-template.h and overwritten by you.
# eg. CONFIG_FILE = ljf-config.h

# CONFIG_FILE = 
