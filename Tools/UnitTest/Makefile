BUILD_DIR := build
TOOL_SOURCES := pubspec.lock $(shell find . -name '*.dart')
BUILD_SNAPSHOT := $(BUILD_DIR)/build.dart.snapshot
TEST_SNAPSHOT := $(BUILD_DIR)/test.dart.snapshot

# Run dart pub get on tool directory.
get:
	@ dart pub get

# Remove all build outputs and intermediate files.
clean:
	@ rm -rf $(BUILD_DIR)
	@ rm -rf gen

$(TEST_SNAPSHOT): $(TOOL_SOURCES)
	@ mkdir -p build
	@ echo "Compiling Dart snapshot..."
	@ dart --snapshot=$@ --snapshot-kind=app-jit bin/test.dart clox
# Run the tests for the final versions of clox and jlox.
test: $(TEST_SNAPSHOT)
	@- dart $(TEST_SNAPSHOT) clox --interpreter ../x64/Release/BytecodeVirtualMachine.exe
	@ dart $(TEST_SNAPSHOT) jlox --interpreter ../x64/Release/TreeWalk.exe

# Run the tests for bytecode.
test_bytecode: $(TEST_SNAPSHOT)
	@ dart $(TEST_SNAPSHOT) clox --interpreter ../x64/Release/BytecodeVirtualMachine.exe

# Run the tests for treewalk.
test_treewalk: $(TEST_SNAPSHOT)
	@ dart $(TEST_SNAPSHOT) jlox --interpreter ../x64/Release/TreeWalk.exe

.PHONY: clean get test test_all test_bytecode test_treewalk