
SOURCE_FILES := $(shell find $(SOURCE_ROOT_DIR)/llcode -name \*.c -or -name \*.cpp)
LL_FILES := $(SOURCE_FILES:%=$(BUILD_DIR)/%.ll)

.PHONY: all

all: $(LL_FILES)

# C ll file
$(BUILD_DIR)/%.c.ll: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -emit-llvm -S $< -o $@ -c

# C++ ll file
$(BUILD_DIR)/%.cpp.ll: %.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -emit-llvm -S $< -o $@ -c
