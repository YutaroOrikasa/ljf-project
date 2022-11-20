# license: CC0

# reference: https://spin.atomicobject.com/2016/08/26/makefile-c-projects/

-include config.mk
export CC CXX

SOURCE_ROOT_DIR ?= .
export SOURCE_ROOT_DIR

SOURCE_DIRS ?= $(SOURCE_ROOT_DIR)
export SOURCE_DIRS

INCLUDE_PATHS ?= ./include ./googletest/googletest/include
export INCLUDE_PATHS

BUILD_DIR ?= build
export BUILD_DIR

LIBLLVM_CXXFLAGS ?= 
export LIBLLVM_CXXFLAGS

LIBLLVM_LDFLAGS ?=  
export LIBLLVM_LDFLAGS


BENCH_NAME ?=
export BENCH_NAME


CFLAGS ?=
CXXFLAGS ?=
LDFLAGS ?=
CONFIG_FILE ?= ljf-config-template.h

override CFLAGS += $(LIBLLVM_CXXFLAGS)
override CXXFLAGS += $(LIBLLVM_CXXFLAGS) -include $(CONFIG_FILE)
override LDFLAGS += $(LIBLLVM_LDFLAGS) -lLLVM
# export doesn't work with overrided the macro
# that is defined on command line argument or given as environment variable,
# so we explicitly have to pass these variables to sub make by commandline argument.

# used by header dependency check in common.mk
SOURCE_FILES := main.cpp ljf.cpp

_DEP_FLAGS := -MMD -MP
INCLUDE_FLAGS := $(INCLUDE_PATHS:%=-I%)
override CFLAGS += -Wall $(INCLUDE_FLAGS) $(_DEP_FLAGS)
override CXXFLAGS += -Wall -std=c++17 $(INCLUDE_FLAGS) $(_DEP_FLAGS)

include common.mk

all:$(BUILD_DIR)/libgtest.a $(BUILD_DIR)/libljf.a $(BUILD_DIR)/main $(BUILD_DIR)/runtime/runtime.so $(BUILD_DIR)/runtime/runtime-declaration.bc

$(BUILD_DIR)/libljf.a: $(BUILD_DIR)/ljf.cpp.o
	mkdir -p $(BUILD_DIR)
	ar -rv $@ $^

$(BUILD_DIR)/main: $(BUILD_DIR)/libljf.a $(BUILD_DIR)/main.cpp.o
	mkdir -p $(BUILD_DIR)
	$(CXX) $(LDFLAGS) $^ -o $@

# runtime.so
# This target have to be PHONY because make can't find header file dependency.
.PHONY: $(BUILD_DIR)/runtime/runtime.so
$(BUILD_DIR)/runtime/runtime.so: $(BUILD_DIR)/libgtest.a
	$(MAKE) -f runtime.mk $(BUILD_DIR)/runtime/runtime.so \
		CFLAGS="$(CFLAGS)" \
		CXXFLAGS="$(CXXFLAGS)" \
		LDFLAGS="$(LDFLAGS)"

# runtime-declaration.bc
$(BUILD_DIR)/runtime/runtime-declaration.bc: runtime/runtime-declaration.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -emit-llvm $^ -o $@

# compile_commands.json
# This target have to be PHONY because make can't find header file dependency.
# NOTEICE: Build with ccache (cache hit) will not work well,
# 			so we  set CXX manually.
.PHONY: compile_commands.json
compile_commands.json: $(BUILD_DIR)/libgtest.a
	$(MAKE) clean
	bear -- $(MAKE) CXX=c++ $(BUILD_DIR)/runtime.so


$(BUILD_DIR)/runtime/unittest-runtime: $(BUILD_DIR)/runtime/runtime.so
	mkdir -p $(BUILD_DIR)
	$(CXX) $(LDFLAGS) $^ -o $@

# gtest
GTEST_DIR := googletest/googletest

$(BUILD_DIR)/libgtest.a: googletest/.git
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -isystem ${GTEST_DIR}/include -I${GTEST_DIR} \
    	-pthread -c ${GTEST_DIR}/src/gtest-all.cc -o $(BUILD_DIR)/gtest-all.o
	ar -rv $(BUILD_DIR)/libgtest.a $(BUILD_DIR)/gtest-all.o

googletest/.git:
	git submodule update --init

# end gtest

.PHONY: benchmark-ll-codes run all-bench pprof-web clean print-source-files

benchmark-ll-codes:
	$(MAKE) -f llcode.mk all \
		CFLAGS="$(CFLAGS)" \
		CXXFLAGS="$(CXXFLAGS)" \
		LDFLAGS="$(LDFLAGS)"

run: all
	$(BUILD_DIR)/$(EXECUTABLE_FILE) "build/llcode/$(BENCH_NAME).cpp.ll" "$(CXXFLAGS) $(LDFLAGS)"

all-bench: all benchmark-ll-codes
	LL_FILES_DIR="$(BUILD_DIR)/llcode" FLAGS="$(CXXFLAGS) $(LDFLAGS)" ./all-bench.sh 

pprof-web:
	# pprof --web build/main tmp/main.prof
	pprof --web build/main tmp/fibo-bigint.prof
	pprof --web build/main tmp/fibo-ljf.prof

run-unittest-runtime: $(BUILD_DIR)/runtime/unittest-runtime
	$(BUILD_DIR)/runtime/unittest-runtime

clean:
	rm -rf build _dump.ll 

print-source-files:
	@echo $(SOURCE_FILES)

