#!/usr/bin/env make -f

### global variables section

# static vars
BUILD_DIR:="build"
BUILD_BIN:="$(BUILD_DIR)/tools/blisp/blisp"

# dynamic vars
FILES_CMAKE:=$(shell find  .  -path ./$(BUILD_DIR) -prune -false  -o  -type f  -name '*.cmake'  -o  -type f  -name 'CMakeLists.txt')
FILES_SRC:=$(shell   find  .  -path ./$(BUILD_DIR) -prune -false  -o  -type f  -name '*.c'      -o  -type f  -name '*.h')

### main targets section

# simplify build
build:  $(FILES_CMAKE)  $(FILES_SRC)  Makefile
	@echo "\n>>>> Generating build files in: $(BUILD_DIR) ...\n"
	@cmake  -S .  -B $(BUILD_DIR)  -DBLISP_BUILD_CLI=ON
	@echo "\n>>>> Building...\n"
	@cmake  --build  $(BUILD_DIR)
	@echo "\n>>>> DONE: $(BUILD_BIN)\n"

# deleting output build directory with its content
clean:
	-@rm -rf $(BUILD_DIR)/

# printf-like debug target
vars:
	@echo "\n>>>> FILES_CMAKE:"
	@echo "$(FILES_CMAKE)" | sed 's, ,\n,g'
	@echo "\n>>>> FILES_SRC:"
	@echo "$(FILES_SRC)" | sed 's, ,\n,g'

.PHONY: clean  vars

