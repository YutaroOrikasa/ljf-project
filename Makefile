# license: CC0

# reference: https://spin.atomicobject.com/2016/08/26/makefile-c-projects/

# Rewrite here for your project
SOURCE_DIRS ?= .
INCLUDE_PATHS ?= ./include
EXECUTABLE_FILE ?= main
BUILD_DIR ?= build
LIBLLVM_CXXFLAGS = -I/usr/local/opt/llvm/include
LIBLLVM_LDFLAGS =  -L/usr/local/opt/llvm/lib -lLLVM

CFLAGS ?=
CXXFLAGS ?=
LDFLAGS ?=
override CFLAGS += $(LIBLLVM_CXXFLAGS)
override CXXFLAGS += $(LIBLLVM_CXXFLAGS)
override LDFLAGS += $(LIBLLVM_LDFLAGS)

# In shell function, escaping is needed sa same as shell script.
SOURCE_FILES := $(shell find $(SOURCE_DIRS) \( -name \*.c -or -name \*.cpp \) -and -not -path ./llcode/\* \
					-and -not -name runtime.cpp)
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

all:$(BUILD_DIR)/$(EXECUTABLE_FILE)

# link
$(BUILD_DIR)/$(EXECUTABLE_FILE): $(OBJECT_FILES) $(LL_FILES) $(BUILD_DIR)/runtime.so
	mkdir -p $(BUILD_DIR)
	$(CXX) $(LDFLAGS) $(OBJECT_FILES) $(BUILD_DIR)/runtime.so -o $@

# runtime.so
$(BUILD_DIR)/runtime.so: $(BUILD_DIR)/runtime.cpp.o
	mkdir -p $(@D)
	$(CXX) $(LDFLAGS) -shared $^ -o $@

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

.PHONY: run pprof-web clean print-source-files

run: $(BUILD_DIR)/$(EXECUTABLE_FILE)
	$(BUILD_DIR)/$(EXECUTABLE_FILE) "build/llcode/tarai.cpp.ll" "$(CXXFLAGS) $(LDFLAGS)"

pprof-web:
	# pprof --web build/main tmp/main.prof
	pprof --web build/main tmp/fibo-bigint.prof
	pprof --web build/main tmp/fibo-ljf.prof

clean:
	rm -rf build tmp _dump.ll 

print-source-files:
	@echo $(SOURCE_FILES)

-include $(DEPENDENCY_FILES)
