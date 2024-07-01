# Shell-Scheduler

## Overview

Shell-Scheduler is a combined implementation of a simple shell and process scheduler written in C programming language. It provides a basic command-line interface for users to execute commands, supports piped commands, maintains a command history, and allows users to submit processes with specified priorities for execution on multiple CPUs using a round-robin scheduling algorithm.

## Features

### Simple Shell
- **Command execution**: Execute single commands entered by the user.
- **Piped commands**: Support for executing commands with pipes.
- **Command history**: Maintains a history of executed commands.
- **Background execution**: Support for running commands in the background using `&`.

### Simple Scheduler
- **Process Scheduling**: Users can submit processes with different priorities for execution.
- **Multiple CPUs**: The program supports scheduling processes on multiple CPUs.
- **Round-Robin Scheduling**: A round-robin scheduling algorithm is used to distribute CPU time among processes.

## Usage

### Compilation

Compile the Shell-Scheduler using the provided Makefile:

```
make
```

### Running the Shell-Scheduler

Execute the compiled binary to start the shell:

```
./simple_shell
```

### Commands

#### Simple Shell Commands
- **Single commands**: Enter any single command to execute.
- **Piped commands**: Use the pipe operator (`|`) to execute piped commands.
- **Background execution**: Append `&` at the end of the command to run it in the background.
- **exit**: Terminate the shell.
- **history**: Display the command history.

#### Simple Scheduler Commands
- **submit program_name priority**: Submit a program for execution with a specified priority (1 to 4).
- **exit**: Exit the shell.

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
