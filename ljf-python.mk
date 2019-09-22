
BUILD_DIR ?= build

-include config.mk

SOURCE_FILES = ljf-python/rtpl.cpp ljf-python/rtpl-phase1.cpp ljf-python/rppl.cpp

EXECUTABLE_FILES = $(SOURCE_FILES:%.cpp=$(BUILD_DIR)/%)

DEPENDENCY_FILES := $(SOURCE_FILES:%.cpp=$(BUILD_DIR)/%.d)


_DEP_FLAGS := -MMD -MP
override CXXFLAGS += -Wall -std=c++17 $(_DEP_FLAGS)

all: $(EXECUTABLE_FILES)

$(BUILD_DIR)/ljf-python/%: ljf-python/%.cpp
	mkdir -p $(BUILD_DIR)/ljf-python
	$(CXX) $(CXXFLAGS) -fno-exceptions $< -o $@

$(BUILD_DIR)/ljf-python/grammar/%.o: ljf-python/grammar/%.cpp
	mkdir -p $(BUILD_DIR)/ljf-python/grammar
	$(CXX) $(CXXFLAGS) -c -fno-exceptions $< -o $@

$(BUILD_DIR)/ljf-python/rppl: $(BUILD_DIR)/ljf-python/grammar/grammar.o ljf-python/rppl.cpp
	mkdir -p $(BUILD_DIR)/ljf-python
	$(CXX) $(CXXFLAGS) -fno-exceptions $(BUILD_DIR)/ljf-python/grammar/grammar.o ljf-python/rppl.cpp -o $@

-include $(DEPENDENCY_FILES)
