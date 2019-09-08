
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
