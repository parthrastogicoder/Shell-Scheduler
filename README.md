# OS-SimpleShell

---

# Simple Shell

Simple Shell is a basic shell implementation written in C programming language. It provides a simple command-line interface for users to execute commands, supports piped commands, and maintains a command history.

## Features

- Command execution: Execute single commands entered by the user.
- Piped commands: Support for executing commands with pipes.
- Command history: Maintains a history of executed commands.
- Background execution: Support for running commands in the background using `&`.

## Usage

### Compilation

Compile the Simple Shell using the provided Makefile:

```
make
```

### Running the Shell

Execute the compiled binary to start the shell:

```
./simple_shell
```

### Commands

The Simple Shell supports the following commands:

- Single commands: Enter any single command to execute.
- Piped commands: Use the pipe operator (`|`) to execute piped commands.
- Background execution: Append `&` at the end of the command to run it in the background.
- `exit`: Terminate the shell.
- `history`: Display the command history.

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

## Contributions

- Aryan Jain
- Parth Sandeep Rastogi
