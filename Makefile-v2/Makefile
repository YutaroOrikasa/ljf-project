
LIBRARY_SOURCE_DIRS ?= runtime
INCLUDE_PATHS ?= ./include ./googletest/googletest/include
EXECUTABLE_SOURCE_DIRS ?=
BUILD_DIR ?= build-v2

# single file libraries/executables
LIBRARY_SOURCE_FILES ?= ljf.cpp
EXECUTABLE_SOURCE_FILES ?= main.cpp

-include ./config.mk

override CFLAGS += $(LIBLLVM_CXXFLAGS)
override CXXFLAGS += $(LIBLLVM_CXXFLAGS) -include $(CONFIG_FILE)
override LDFLAGS += $(LIBLLVM_LDFLAGS) -lLLVM


# string replacement
# e.g aaa -> $(BUILD_DIR)/aaa.o
# % maches 'aaa' in example.
LIBRARY_TARGETS := $(LIBRARY_SOURCE_DIRS:%=$(BUILD_DIR)/%.a)
EXECUTABLE_TARGETS := $(EXECUTABLE_SOURCE_DIRS:%=$(BUILD_DIR)/%)
SINGLE_SOURCE_LIBRARY_TARGETS := $(addsuffix .a, $(basename $(LIBRARY_SOURCE_FILES:%=$(BUILD_DIR)/%)))
SINGLE_SOURCE_EXECUTABLE_TARGETS := $(basename $(EXECUTABLE_SOURCE_FILES:%=$(BUILD_DIR)/%))

ALL_LIBRARY_TARGETS := $(SINGLE_SOURCE_LIBRARY_TARGETS) $(LIBRARY_TARGETS)


_DEP_FLAGS := -MMD -MP
INCLUDE_FLAGS := $(INCLUDE_PATHS:%=-I%)
override CFLAGS += -Wall $(INCLUDE_FLAGS) $(_DEP_FLAGS)
override CXXFLAGS += -Wall -std=c++17 $(INCLUDE_FLAGS) $(_DEP_FLAGS)


export CFLAGS
export CXXFLAGS
export LDFLAGS
export BUILD_DIR

all: $(LIBRARY_TARGETS) $(EXECUTABLE_TARGETS) \
	$(SINGLE_SOURCE_LIBRARY_TARGETS) $(SINGLE_SOURCE_EXECUTABLE_TARGETS) \
	$(BUILD_DIR)/runtime.so

# runtime.so
$(BUILD_DIR)/runtime.so: $(BUILD_DIR)/runtime.a
	mkdir -p $(@D)
	$(CXX) $(LDFLAGS) -shared $^ -o $@


$(SINGLE_SOURCE_LIBRARY_TARGETS): FORCE
	@echo SINGLE LIB $@
	$(MAKE) -f Makefile-v2/submake.mk TARGET=$(@) SINGLE_SOURCE=1 BUILD_DIR=$(BUILD_DIR)

$(SINGLE_SOURCE_EXECUTABLE_TARGETS): $(ALL_LIBRARY_TARGETS) FORCE
	@echo SINGLE $@
	$(MAKE) -f Makefile-v2/submake.mk TARGET=$(@) SINGLE_SOURCE=1 \
		LIBRARY_TARGETS="$(ALL_LIBRARY_TARGETS)" BUILD_DIR=$(BUILD_DIR)


$(BUILD_DIR)/%.a: FORCE
	$(MAKE) -f Makefile-v2/submake.mk TARGET=$(@) BUILD_DIR=$(BUILD_DIR)

$(BUILD_DIR)/%: $(ALL_LIBRARY_TARGETS) FORCE
	$(MAKE) -f Makefile-v2/submake.mk TARGET=$(@) LIBRARY_TARGETS="$(ALL_LIBRARY_TARGETS)" BUILD_DIR=$(BUILD_DIR)

FORCE: ;

# PHONY targets
.PHONY: list-targets test

list-targets:
	@printf '%s\n' $(LIBRARY_TARGETS) $(EXECUTABLE_TARGETS)


.PHONY: benchmark-ll-codes run all-bench pprof-web clean print-source-files


benchmark-ll-codes:
	$(MAKE) -f llcode.mk all \
		SOURCE_ROOT_DIR="." \
		CFLAGS="$(CFLAGS)" \
		CXXFLAGS="$(CXXFLAGS)" \
		LDFLAGS="$(LDFLAGS)"

run: all $(BUILD_DIR)/llcode/$(BENCH_NAME).cpp.ll
	$(BUILD_DIR)/main "$(BUILD_DIR)/llcode/$(BENCH_NAME).cpp.ll" "$(CXXFLAGS) $(LDFLAGS)"

all-bench: all benchmark-ll-codes
	LL_FILES_DIR="$(BUILD_DIR)/llcode" FLAGS="$(CXXFLAGS) $(LDFLAGS)" ./all-bench.sh

pprof-web:
	# pprof --web build/main tmp/main.prof
	pprof --web build/main tmp/fibo-bigint.prof
	pprof --web build/main tmp/fibo-ljf.prof

run-unittest-runtime: $(BUILD_DIR)/unittest-runtime
	$(BUILD_DIR)/unittest-runtime

clean:
	rm -rf $(BUILD_DIR) _dump.ll

# print-source-files:
# 	@echo $(SOURCE_FILES)
