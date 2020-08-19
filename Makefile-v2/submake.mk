# license: CC0

# processed on sub make execution

TARGET_NAME := $(TARGET:%.a=%)
SOURCE_DIR := $(TARGET_NAME:$(BUILD_DIR)/%=%)

LIBRARY_NAME := $(SOURCE_DIR)
LIBRARY_FILE := $(LIBRARY_NAME:%=%.a)
EXECUTABLE_FILE := $(SOURCE_DIR)

all: $(TARGET)

# In shell function, escaping is needed sa same as shell script.
SOURCE_FILES := $(shell if [ "$(SINGLE_SOURCE)" = 1 ];then \
	echo $(SOURCE_DIR).*; \
else \
	find "$(SOURCE_DIR)" -name \*.c -or -name \*.cpp; \
fi)

OBJECT_FILES := $(SOURCE_FILES:%=$(BUILD_DIR)/_build/%.o)

# C file
$(BUILD_DIR)/_build/%.c.o: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $< -o $@ -c

# C++ file
$(BUILD_DIR)/_build/%.cpp.o: %.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $< -o $@ -c


$(BUILD_DIR)/$(LIBRARY_NAME).a: $(OBJECT_FILES)
	mkdir -p $(@D)
	ar rsc $@ $(OBJECT_FILES)

$(BUILD_DIR)/$(EXECUTABLE_FILE): $(OBJECT_FILES)
	mkdir -p $(@D)
	$(CXX) $(LDFLAGS) $(OBJECT_FILES) $(LIBRARY_TARGETS) -o $@


DEPENDENCY_FILES := $(OBJECT_FILES:%.o=%.d)
-include $(DEPENDENCY_FILES)
