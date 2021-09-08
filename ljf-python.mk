
BUILD_DIR ?= build

-include config.mk

INCLUDE_PATHS = ./include ./googletest/googletest/include ./
INCLUDE_FLAGS := $(INCLUDE_PATHS:%=-I%)

_DEP_FLAGS := -MMD -MP
override CXXFLAGS += -Wall -std=c++17 $(_DEP_FLAGS) $(INCLUDE_FLAGS) -fno-exceptions


### first rule ###

all: _all

### grammar impl ###
GRAMMAR_SOURCE_FILES = $(shell find ljf-python/grammar -name '*.cpp')
GRAMMAR_OBJECT_FILES = $(GRAMMAR_SOURCE_FILES:%=$(BUILD_DIR)/%.o)


### executable ###

EXE_SOURCE_FILES = ljf-python/rtpl.cpp ljf-python/rtpl-phase1.cpp ljf-python/rppl.cpp
EXECUTABLE_FILES = $(EXE_SOURCE_FILES:ljf-python/%.cpp=$(BUILD_DIR)/ljf-python/bin/%)

$(BUILD_DIR)/ljf-python/bin/%: $(BUILD_DIR)/ljf-python/%.cpp.o
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BUILD_DIR)/ljf-python/%.cpp.o: ljf-python/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@


$(BUILD_DIR)/ljf-python/bin/rppl: $(BUILD_DIR)/ljf-python/rppl.cpp.o $(GRAMMAR_OBJECT_FILES)
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(BUILD_DIR)/ljf-python/rppl.cpp.o $(GRAMMAR_OBJECT_FILES) -o $@


### run executable ###

.PHONY: run-rtpl
run-rtpl: $(BUILD_DIR)/ljf-python/bin/rtpl
	$(BUILD_DIR)/ljf-python/bin/rtpl

.PHONY: run-rppl
run-rppl: $(BUILD_DIR)/ljf-python/bin/rppl
	$(BUILD_DIR)/ljf-python/bin/rppl


### unittest ###

UNITTEST_SOURCE_FILES = $(shell find ljf-python/unittests -name '*.cpp')
UNITTEST_OBJECT_FILES = $(UNITTEST_SOURCE_FILES:%=$(BUILD_DIR)/%.o)
UNITTEST_EXECUTABLE = $(BUILD_DIR)/ljf-python/unittest

$(BUILD_DIR)/ljf-python/unittests/%.cpp.o: ljf-python/unittests/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(UNITTEST_EXECUTABLE): $(UNITTEST_OBJECT_FILES) $(GRAMMAR_OBJECT_FILES)
	mkdir -p $(@D)
	$(MAKE) $(BUILD_DIR)/libgtest.a
	$(CXX) $(CXXFLAGS) $(BUILD_DIR)/libgtest.a \
										$(UNITTEST_OBJECT_FILES) \
										$(GRAMMAR_OBJECT_FILES) -o $@

.PHONY: run-unittest
run-unittest: $(UNITTEST_EXECUTABLE)
	$(UNITTEST_EXECUTABLE)


### header dependency ###

SOURCE_FILES = $(EXE_SOURCE_FILES) $(GRAMMAR_SOURCE_FILES) $(UNITTEST_SOURCE_FILES)
DEPENDENCY_FILES := $(SOURCE_FILES:%=$(BUILD_DIR)/%.d)
-include $(DEPENDENCY_FILES)


### all ###
# body of rule 'all'
.PHONY: _all
_all: $(EXECUTABLE_FILES) $(UNITTEST_EXECUTABLE)
