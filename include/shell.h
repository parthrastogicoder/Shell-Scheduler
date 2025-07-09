#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>

#define MAX_SIZE 1024
#define MAX_HIS 200

// Structure to store command history
typedef struct {
    char cmd[MAX_SIZE];
    pid_t pid;
    time_t start_time;
    int background;
} HistoryEntry;

// Function declarations
int create_process_and_run(char *cmd, int background);
int execute_piped_commands(char *cmd);
void sigint_handler(int signo);
void setup_signal_handler(void);
void shell_loop(void);
int launch(char *cmd);
void display_history(void);
void trim_whitespace(char *str);
int history_piped_commands(char *cmd);
int run_shell(void); // For integration

#endif // SHELL_H
