# Makefile for Shell-Scheduler Project
# Developed by Aryan Jain & Parth Rastogi - Group 8 Sec A

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread -D_GNU_SOURCE
LDFLAGS = -lpthread

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin
TEST_DIR = tests

# Source files
SHELL_SRC = $(SRC_DIR)/shell.c
BASIC_SCHEDULER_SRC = $(SRC_DIR)/basic_scheduler.c
ADVANCED_SCHEDULER_SRC = $(SRC_DIR)/advanced_scheduler.c
INTEGRATED_SHELL_SRC = $(SRC_DIR)/integrated_shell.c
GLOBALS_SRC = $(SRC_DIR)/globals.c
SCHEDULER_COMMON_SRC = $(SRC_DIR)/scheduler_common.c

# Object files
SHELL_OBJ = $(BUILD_DIR)/shell.o
BASIC_SCHEDULER_OBJ = $(BUILD_DIR)/basic_scheduler.o
ADVANCED_SCHEDULER_OBJ = $(BUILD_DIR)/advanced_scheduler.o
INTEGRATED_SHELL_OBJ = $(BUILD_DIR)/integrated_shell.o
GLOBALS_OBJ = $(BUILD_DIR)/globals.o
SCHEDULER_COMMON_OBJ = $(BUILD_DIR)/scheduler_common.o

# Executables
SHELL_EXEC = $(BIN_DIR)/simple_shell
BASIC_SCHEDULER_EXEC = $(BIN_DIR)/basic_scheduler
ADVANCED_SCHEDULER_EXEC = $(BIN_DIR)/advanced_scheduler
INTEGRATED_SHELL_EXEC = $(BIN_DIR)/integrated_shell

# Default target
all: directories $(SHELL_EXEC) $(BASIC_SCHEDULER_EXEC) $(ADVANCED_SCHEDULER_EXEC) $(INTEGRATED_SHELL_EXEC)

# Create necessary directories
directories:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR)

# Compile individual object files
$(SHELL_OBJ): $(SHELL_SRC)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(BASIC_SCHEDULER_OBJ): $(BASIC_SCHEDULER_SRC)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(ADVANCED_SCHEDULER_OBJ): $(ADVANCED_SCHEDULER_SRC)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(INTEGRATED_SHELL_OBJ): $(INTEGRATED_SHELL_SRC)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(GLOBALS_OBJ): $(GLOBALS_SRC)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(SCHEDULER_COMMON_OBJ): $(SCHEDULER_COMMON_SRC)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Special object files for integration (with INTEGRATED_BUILD flag)
$(BUILD_DIR)/shell_integrated.o: $(SHELL_SRC)
	$(CC) $(CFLAGS) -DINTEGRATED_BUILD -I$(INCLUDE_DIR) -c $< -o $@

$(BUILD_DIR)/basic_scheduler_integrated.o: $(BASIC_SCHEDULER_SRC)
	$(CC) $(CFLAGS) -DINTEGRATED_BUILD -I$(INCLUDE_DIR) -c $< -o $@

$(BUILD_DIR)/advanced_scheduler_integrated.o: $(ADVANCED_SCHEDULER_SRC)
	$(CC) $(CFLAGS) -DINTEGRATED_BUILD -I$(INCLUDE_DIR) -c $< -o $@

# Build executables
$(SHELL_EXEC): $(SHELL_OBJ)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

$(BASIC_SCHEDULER_EXEC): $(BASIC_SCHEDULER_OBJ) $(GLOBALS_OBJ) $(SCHEDULER_COMMON_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(ADVANCED_SCHEDULER_EXEC): $(ADVANCED_SCHEDULER_OBJ) $(GLOBALS_OBJ) $(SCHEDULER_COMMON_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(INTEGRATED_SHELL_EXEC): $(INTEGRATED_SHELL_OBJ) $(BUILD_DIR)/shell_integrated.o $(BUILD_DIR)/basic_scheduler_integrated.o $(BUILD_DIR)/advanced_scheduler_integrated.o $(GLOBALS_OBJ) $(SCHEDULER_COMMON_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Individual targets
shell: directories $(SHELL_EXEC)
basic_scheduler: directories $(BASIC_SCHEDULER_EXEC)
advanced_scheduler: directories $(ADVANCED_SCHEDULER_EXEC)
integrated: directories $(INTEGRATED_SHELL_EXEC)

# Test targets
test: all
	@echo "Running tests..."
	@chmod +x $(TEST_DIR)/*.sh
	@$(TEST_DIR)/run_tests.sh

# Clean build files
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Install (copy to system PATH)
install: all
	@echo "Installing executables..."
	sudo cp $(BIN_DIR)/* /usr/local/bin/
	@echo "Installation complete!"

# Uninstall
uninstall:
	@echo "Uninstalling executables..."
	sudo rm -f /usr/local/bin/simple_shell
	sudo rm -f /usr/local/bin/basic_scheduler
	sudo rm -f /usr/local/bin/advanced_scheduler
	sudo rm -f /usr/local/bin/integrated_shell
	@echo "Uninstallation complete!"

# Run the integrated shell
run: integrated
	@echo "Starting Integrated Shell-Scheduler..."
	@$(INTEGRATED_SHELL_EXEC)

# Run individual components
run-shell: shell
	@echo "Starting Simple Shell..."
	@$(SHELL_EXEC)

run-basic: basic_scheduler
	@echo "Starting Basic Scheduler..."
	@$(BASIC_SCHEDULER_EXEC)

run-advanced: advanced_scheduler
	@echo "Starting Advanced Scheduler..."
	@$(ADVANCED_SCHEDULER_EXEC)

# Debug versions
debug: CFLAGS += -g -DDEBUG
debug: all

# Help
help:
	@echo "Available targets:"
	@echo "  all           - Build all executables"
	@echo "  shell         - Build simple shell only"
	@echo "  basic_scheduler - Build basic scheduler only"
	@echo "  advanced_scheduler - Build advanced scheduler only"
	@echo "  integrated    - Build integrated shell only"
	@echo "  run           - Run integrated shell"
	@echo "  run-shell     - Run simple shell"
	@echo "  run-basic     - Run basic scheduler"
	@echo "  run-advanced  - Run advanced scheduler"
	@echo "  test          - Run all tests"
	@echo "  clean         - Remove build files"
	@echo "  install       - Install to system PATH"
	@echo "  uninstall     - Remove from system PATH"
	@echo "  debug         - Build with debug symbols"
	@echo "  help          - Show this help"

.PHONY: all directories shell basic_scheduler advanced_scheduler integrated test clean install uninstall run run-shell run-basic run-advanced debug help
