
BUILD_DIR ?= build

-include config.mk

INCLUDE_PATHS = ./include ./googletest/googletest/include ./
INCLUDE_FLAGS := $(INCLUDE_PATHS:%=-I%)

EXE_SOURCE_FILES = ljf-python/rtpl.cpp ljf-python/rtpl-phase1.cpp ljf-python/rppl.cpp 

GRAMMAR_SOURCE_FILES = $(shell find ljf-python/grammar -name '*.cpp')

UNITTEST_SOURCE_FILES = $(shell find ljf-python/unittests -name '*.cpp')

SOURCE_FILES = $(EXE_SOURCE_FILES) $(GRAMMAR_SOURCE_FILES) $(UNITTEST_SOURCE_FILES)

UNITTEST_OBJECT_FILES = $(UNITTEST_SOURCE_FILES:%=$(BUILD_DIR)/%.o)
UNITTEST_EXECUTABLE = $(BUILD_DIR)/ljf-python/unittest

EXECUTABLE_FILES = $(EXE_SOURCE_FILES:ljf-python/%.cpp=$(BUILD_DIR)/ljf-python/bin/%)

DEPENDENCY_FILES := $(SOURCE_FILES:%=$(BUILD_DIR)/%.d)


_DEP_FLAGS := -MMD -MP
override CXXFLAGS += -Wall -std=c++17 $(_DEP_FLAGS) $(INCLUDE_FLAGS)

all: $(EXECUTABLE_FILES)

$(BUILD_DIR)/ljf-python/bin/%: $(BUILD_DIR)/ljf-python/%.o
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -fno-exceptions $< -o $@

$(BUILD_DIR)/ljf-python/%.cpp.o: ljf-python/%.cpp
	mkdir -p $(BUILD_DIR)/ljf-python
	$(CXX) $(CXXFLAGS) -c -fno-exceptions $< -o $@

# $(BUILD_DIR)/ljf-python/rppl: \
# 							$(BUILD_DIR)/ljf-python/grammar/grammar.o \
# 							$(BUILD_DIR)/ljf-python/grammar/expr.o \
# 							ljf-python/rppl.cpp
# 	mkdir -p $(BUILD_DIR)/ljf-python
# 	$(CXX) $(CXXFLAGS) -fno-exceptions $(BUILD_DIR)/ljf-python/grammar/grammar.o \
# 										$(BUILD_DIR)/ljf-python/grammar/expr.o \
# 										ljf-python/rppl.cpp -o $@

$(BUILD_DIR)/ljf-python/bin/rppl: $(BUILD_DIR)/ljf-python/rppl.cpp.o
	mkdir -p $(BUILD_DIR)/ljf-python
	$(CXX) $(CXXFLAGS) -fno-exceptions $(BUILD_DIR)/ljf-python/rppl.cpp.o -o $@

$(BUILD_DIR)/ljf-python/unittests/%.cpp.o: ljf-python/unittests/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -fno-exceptions $< -o $@

$(UNITTEST_EXECUTABLE): $(UNITTEST_OBJECT_FILES)
	mkdir -p $(@D)
	$(MAKE) $(BUILD_DIR)/libgtest.a
	$(CXX) $(CXXFLAGS) -fno-exceptions $(BUILD_DIR)/libgtest.a $(UNITTEST_OBJECT_FILES) -o $@

.PHONY: run-unittest
run-unittest: $(UNITTEST_EXECUTABLE)
	$(UNITTEST_EXECUTABLE)

-include $(DEPENDENCY_FILES)
