# Shell-Scheduler

## Overview

Shell-Scheduler is a comprehensive implementation of a simple shell and process scheduler system written in C programming language. It provides a modular architecture with three operation modes: a basic shell, a round-robin scheduler, and an advanced priority-based scheduler. The system supports command execution, piped commands, command history, background processes, and multi-CPU process scheduling.

**Developed by:** Aryan Jain & Parth Rastogi - Group 8 Sec A

## Project Structure

```
Shell-Scheduler/
├── include/                 # Header files
│   ├── shell.h             # Shell function declarations
│   ├── scheduler.h         # Scheduler function declarations
│   └── integrated_shell.h  # Integration declarations
├── src/                    # Source files
│   ├── shell.c            # Simple shell implementation
│   ├── basic_scheduler.c   # Basic round-robin scheduler
│   ├── advanced_scheduler.c # Priority-based scheduler
│   └── integrated_shell.c  # Main integration program
├── tests/                  # Test scripts
│   ├── run_tests.sh       # Main test runner
│   ├── test_shell.sh      # Shell tests
│   ├── test_basic_scheduler.sh    # Basic scheduler tests
│   ├── test_advanced_scheduler.sh # Advanced scheduler tests
│   └── test_integrated_shell.sh   # Integration tests
├── bin/                    # Compiled executables (created during build)
├── build/                  # Object files (created during build)
├── Makefile               # Build configuration
└── README.md              # This file
```

## Features

### 1. Simple Shell
- **Command execution**: Execute single commands entered by the user
- **Piped commands**: Support for executing commands with pipes (`|`)
- **Command history**: Maintains a history of executed commands
- **Background execution**: Support for running commands in the background using `&`
- **Signal handling**: Proper handling of Ctrl+C with graceful shutdown
- **Process management**: Fork-exec model for command execution

### 2. Basic Scheduler (Round-Robin)
- **Round-Robin Scheduling**: Time-sliced process execution
- **Multiple CPUs**: Configurable number of CPUs for parallel execution
- **Process Queue Management**: Shared memory-based process queues
- **Process State Tracking**: Running, waiting, and terminated states
- **Execution Time Monitoring**: Track execution and waiting times
- **Signal-based Communication**: SIGUSR1 and SIGCHLD handling

### 3. Advanced Scheduler (Priority-based)
- **Priority Levels**: 4 priority levels (1-highest, 4-lowest)
- **Priority-based Time Slicing**: Higher priority processes get more CPU time
- **Preemptive Scheduling**: Lower priority processes can be preempted
- **Priority Queue Management**: Separate handling for each priority level
- **Enhanced Process Control**: More sophisticated process state management

### 4. Integrated System
- **Menu-driven Interface**: Easy selection between different modes
- **Seamless Integration**: All components work together
- **Help System**: Comprehensive help documentation
- **Error Handling**: Robust error handling and reporting

## Compilation and Installation

### Prerequisites
- GCC compiler
- POSIX-compliant system (Linux/Unix)
- pthread library
- Standard C library

### Building the Project

```bash
# Build all components
make all

# Build individual components
make shell                 # Simple shell only
make basic_scheduler       # Basic scheduler only
make advanced_scheduler    # Advanced scheduler only
make integrated           # Integrated system only

# Build with debug symbols
make debug

# Clean build files
make clean
```

### Installation (Optional)

```bash
# Install to system PATH
make install

# Uninstall from system PATH
make uninstall
```

## Usage

### Running the Integrated System

```bash
# Run the integrated shell (recommended)
make run

# Or run directly
./bin/integrated_shell
```

### Running Individual Components

```bash
# Run simple shell
make run-shell
# Or: ./bin/simple_shell

# Run basic scheduler
make run-basic
# Or: ./bin/basic_scheduler

# Run advanced scheduler
make run-advanced
# Or: ./bin/advanced_scheduler
```

## Commands and Usage

### Simple Shell Commands
- **Single commands**: Enter any system command to execute
  ```bash
  ls -la
  pwd
  date
  ```
- **Piped commands**: Use the pipe operator (`|`) to chain commands
  ```bash
  ls -la | grep ".c"
  ps aux | grep shell
  ```
- **Background execution**: Append `&` to run commands in background
  ```bash
  sleep 10 &
  ./long_running_program &
  ```
- **Built-in commands**:
  - `history`: Display command history
  - `exit`: Exit the shell
  - `Ctrl+C`: Exit with history display

### Basic Scheduler Commands
1. **Initial Setup**: Enter number of CPUs and time quantum
   ```
   Enter the number of CPUs: 2
   Enter the time quantum (TSLICE) in milliseconds: 100
   ```

2. **Process Submission**: Submit programs for execution
   ```bash
   submit ./my_program
   submit /bin/ls
   ```

3. **Other Commands**:
   - `exit`: Exit the scheduler
   - Any system command: Execute directly

### Advanced Scheduler Commands
1. **Initial Setup**: Same as basic scheduler
   ```
   Enter the number of CPUs: 2
   Enter the time quantum (TSLICE) in milliseconds: 100
   ```

2. **Priority-based Process Submission**:
   ```bash
   submit ./high_priority_program 1    # Highest priority
   submit ./medium_priority_program 2  # Medium priority
   submit ./low_priority_program 4     # Lowest priority
   submit ./default_program            # Default priority (1)
   ```

3. **Priority Levels**:
   - `1`: Highest priority (gets full time slice)
   - `2`: High priority (gets 1/2 time slice)
   - `3`: Medium priority (gets 1/3 time slice)
   - `4`: Lowest priority (gets 1/4 time slice)

## Testing

### Running All Tests
```bash
make test
```

### Running Individual Tests
```bash
# Test shell functionality
./tests/test_shell.sh

# Test basic scheduler
./tests/test_basic_scheduler.sh

# Test advanced scheduler
./tests/test_advanced_scheduler.sh

# Test integrated system
./tests/test_integrated_shell.sh
```

## Architecture Details

### Process Management
- **Fork-Exec Model**: Each command/program runs in a separate process
- **Signal Handling**: Proper handling of SIGCHLD, SIGUSR1, SIGINT, SIGSTOP, SIGCONT
- **Shared Memory**: IPC using shared memory for process queues
- **Semaphores**: Thread-safe operations using POSIX semaphores

### Scheduling Algorithms
- **Round-Robin**: Equal time slices for all processes
- **Priority-based**: Time slice allocation based on priority levels
- **Preemptive**: Higher priority processes can interrupt lower priority ones

### Memory Management
- **Shared Memory**: Process queues stored in shared memory
- **Proper Cleanup**: Automatic cleanup of shared memory and semaphores
- **Memory Safety**: Bounds checking and safe string operations

## Error Handling

The system includes comprehensive error handling for:
- Invalid commands and arguments
- Process creation failures
- Memory allocation failures
- Signal handling errors
- File descriptor errors
- Semaphore operations

## Limitations and Considerations

1. **Platform Dependency**: Designed for POSIX-compliant systems
2. **Resource Limits**: Limited by system resources (memory, process limits)
3. **Signal Handling**: May behave differently on different Unix variants
4. **Priority Inversion**: Advanced scheduler may experience priority inversion
5. **Starvation**: Lower priority processes may starve in heavy load conditions

## Future Enhancements

- **Dynamic Priority Adjustment**: Implement aging to prevent starvation
- **GUI Interface**: Add graphical user interface
- **Network Support**: Distributed scheduling across multiple machines
- **Advanced Algorithms**: Implement more sophisticated scheduling algorithms
- **Performance Monitoring**: Add detailed performance metrics and monitoring
- **Configuration Files**: Support for configuration files
- **Plugin System**: Extensible architecture for custom schedulers

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass
6. Submit a pull request

## License

This project is developed as part of an academic assignment by Aryan Jain & Parth Rastogi - Group 8 Sec A.

## Contact

For questions or issues, please contact the developers:
- Aryan Jain
- Parth Rastogi

---

**Note**: This project is designed for educational purposes and demonstrates concepts in operating systems, process management, and system programming.

## Examples

### Single Command Execution

```
$ ls -l
```

### Piped Commands

```
$ ls -l | grep .txt
```

### Background Execution

```
$ sleep 10 &
```

### Command History

To display the command history, enter:

```
$ history
```

### Process Submission

```
$ submit my_program 2
```

## Notes

### Simple Scheduler Specifics
- **CPU Configuration**: Before using the program, you need to specify the number of CPUs and the time quantum (TSLICE) in milliseconds.
- **Signals**: The program uses signals (SIGUSR1 and SIGCHLD) for process management and scheduling.

## Contributions

- Parth Sandeep Rastogi
- Aryan Jain
