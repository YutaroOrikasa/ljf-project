
SOURCE_FILES := $(shell find $(SOURCE_ROOT_DIR)/runtime -name \*.c -or -name \*.cpp \! -path './runtime/unittests/*')

include common.mk

# runtime.so
$(BUILD_DIR)/runtime/runtime.so: $(SOURCE_FILES:%=$(BUILD_DIR)/%.o)
	mkdir -p $(@D)
	$(CXX) $(LDFLAGS) $(BUILD_DIR)/libgtest.a -shared $^ -o $@
