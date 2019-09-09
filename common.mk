
DEPENDENCY_FILES := $(SOURCE_FILES:%=$(BUILD_DIR)/%.d)
-include $(DEPENDENCY_FILES)

# C file
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $< -o $@ -c

# C++ file
$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -g $< -o $@ -c

