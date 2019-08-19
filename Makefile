# license: CC0

# reference: https://spin.atomicobject.com/2016/08/26/makefile-c-projects/

# Rewrite here for your project
SOURCE_DIRS ?= .
INCLUDE_PATHS ?= ./include ./googletest/googletest/include
EXECUTABLE_FILE ?= main
BUILD_DIR ?= build
LIBLLVM_CXXFLAGS = -I/usr/local/opt/llvm/include
LIBLLVM_LDFLAGS =  -L/usr/local/opt/llvm/lib -lLLVM

BENCH_NAME ?=

CFLAGS ?=
CXXFLAGS ?=
LDFLAGS ?=
CONFIG_FILE ?= ljf-config-template.h
override CFLAGS += $(LIBLLVM_CXXFLAGS)
override CXXFLAGS += $(LIBLLVM_CXXFLAGS) -include $(CONFIG_FILE)
override LDFLAGS += $(LIBLLVM_LDFLAGS)

# In shell function, escaping is needed sa same as shell script.
SOURCE_FILES := $(shell find $(SOURCE_DIRS) \( -name \*.c -or -name \*.cpp \) -and -not -path ./llcode/\* \
					-and -not -path ./runtime/\*)
RUNTIME_SOURCE_FILES := $(shell find $(SOURCE_DIRS)/runtime -name \*.c -or -name \*.cpp)
LL_SOURCE_FILES := $(shell find $(SOURCE_DIRS) \( -name \*.c -or -name \*.cpp \) -and -path ./llcode/\*)

# string replacement
# e.g aaa -> $(BUILD_DIR)/aaa.o
# % maches 'aaa' in example.
OBJECT_FILES := $(SOURCE_FILES:%=$(BUILD_DIR)/%.o)
LL_FILES := $(LL_SOURCE_FILES:%=$(BUILD_DIR)/%.ll)
DEPENDENCY_FILES := $(OBJECT_FILES:%.o=%.d) $(LL_FILES:%.ll=%.d)

_DEP_FLAGS := -MMD -MP
INCLUDE_FLAGS := $(INCLUDE_PATHS:%=-I%)
override CFLAGS += -Wall $(INCLUDE_FLAGS) $(_DEP_FLAGS)
override CXXFLAGS += -Wall -std=c++17 $(INCLUDE_FLAGS) $(_DEP_FLAGS)

all:$(BUILD_DIR)/libgtest.a $(BUILD_DIR)/$(EXECUTABLE_FILE) $(BUILD_DIR)/runtime.so

# link
$(BUILD_DIR)/$(EXECUTABLE_FILE): $(OBJECT_FILES) $(LL_FILES)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(LDFLAGS) $(OBJECT_FILES) -o $@

# runtime.so
$(BUILD_DIR)/runtime.so: $(RUNTIME_SOURCE_FILES:%=$(BUILD_DIR)/%.o)
	mkdir -p $(@D)
	$(CXX) $(LDFLAGS) $(BUILD_DIR)/libgtest.a -shared $^ -o $@

$(BUILD_DIR)/unittest-runtime: $(BUILD_DIR)/runtime.so
	mkdir -p $(BUILD_DIR)
	$(CXX) $(LDFLAGS) $^ -o $@

# C file
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $< -o $@ -c

# C++ file
$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -g $< -o $@ -c

# C ll file
$(BUILD_DIR)/%.c.ll: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -emit-llvm -S $< -o $@ -c

# C++ ll file
$(BUILD_DIR)/%.cpp.ll: %.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -emit-llvm -S $< -o $@ -c

# gtest
GTEST_DIR := googletest/googletest

$(BUILD_DIR)/libgtest.a: googletest/.git
	$(CXX) $(CXXFLAGS) -isystem ${GTEST_DIR}/include -I${GTEST_DIR} \
    	-pthread -c ${GTEST_DIR}/src/gtest-all.cc -o $(BUILD_DIR)/gtest-all.o
	ar -rv $(BUILD_DIR)/libgtest.a $(BUILD_DIR)/gtest-all.o

googletest/.git:
	git submodule update --init

# end gtest

.PHONY: run all-bench pprof-web clean print-source-files

run: all
	$(BUILD_DIR)/$(EXECUTABLE_FILE) "build/llcode/$(BENCH_NAME).cpp.ll" "$(CXXFLAGS) $(LDFLAGS)"

all-bench: all
	BUILD_DIR="$(BUILD_DIR)" LL_FILES="$(LL_FILES)" FLAGS="$(CXXFLAGS) $(LDFLAGS)" ./all-bench.sh 

pprof-web:
	# pprof --web build/main tmp/main.prof
	pprof --web build/main tmp/fibo-bigint.prof
	pprof --web build/main tmp/fibo-ljf.prof

run-unittest-runtime: $(BUILD_DIR)/unittest-runtime
	$(BUILD_DIR)/unittest-runtime

clean:
	rm -rf build _dump.ll 

print-source-files:
	@echo $(SOURCE_FILES)

-include $(DEPENDENCY_FILES)
