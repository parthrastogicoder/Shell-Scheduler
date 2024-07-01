# OS-SimpleScheduler
---

# SimpleScheduler

## Overview
SimpleScheduler is a simple process scheduling program implemented in C. It allows users to submit processes with specified priorities for execution on multiple CPUs, using a round-robin scheduling algorithm.

## Features
- **Process Scheduling**: Users can submit processes with different priorities for execution.
- **Multiple CPUs**: The program supports scheduling processes on multiple CPUs.
- **Round-Robin Scheduling**: A round-robin scheduling algorithm is used to distribute CPU time among processes.

## Usage
1. **Compilation**: Compile the program using a C compiler (e.g., gcc).
    ```
    gcc -o SimpleScheduler SimpleScheduler.c -lrt -pthread
    ```
2. **Execution**: Run the compiled program.
    ```
    ./SimpleScheduler
    ```
3. **Commands**:
    - `submit program_name priority`: Submit a program for execution with a specified priority (1 to 4).
    - `exit`: Exit the shell.

## Notes
- **CPU Configuration**: Before using the program, you need to specify the number of CPUs and the time quantum (TSLICE) in milliseconds.
- **Signals**: The program uses signals (SIGUSR1 and SIGCHLD) for process management and scheduling.

## Authors
- Aryan Jain
- Parth Rastogi

---
