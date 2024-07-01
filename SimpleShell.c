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
typedef struct
{
    char cmd[MAX_SIZE];
    pid_t pid;
    time_t start_time;
    int background;
} HistoryEntry;

HistoryEntry history[MAX_HIS];
int history_count = 0;

// Function to create a child process and run a command
int create_process_and_run(char *cmd, int background);

// Function to handle piped commands
int execute_piped_commands(char *cmd);

// Signal handler for Ctrl+C
void sigint_handler(int signo);

// Setup signal handler for Ctrl+C
void setup_signal_handler();

// Main shell loop
void shell_loop();

// Launch a command
int launch(char *cmd);

// Display command history
void display_history();

int main()
{
    setup_signal_handler();
    shell_loop();
    return 0;
}

// Function to create a child process and run a command
int create_process_and_run(char *cmd, int background)
{
    int status;
    pid_t child_pid = fork();

    if (child_pid < 0)
    {
        perror("Fork error");
        exit(EXIT_FAILURE);
    }
    else if (child_pid == 0)
    { // Child process
        if (background)
        {
            if (setsid() == -1)
            {
                perror("setsid error");
                exit(EXIT_FAILURE);
            }
        }

        char *args[MAX_SIZE];
        char *token = strtok(cmd, " ");
        int i = 0;

        while (token != NULL)
        {
            args[i++] = token;
            token = strtok(NULL, " ");
        }

        args[i] = NULL;

        if (execvp(args[0], args) == -1)
        {
            perror("Exec error");
            exit(EXIT_FAILURE);
        }
    }
    else
    { // Parent process
        if (!background)
        {
            int status;
            if (waitpid(child_pid, &status, 0) == -1)
            {
                perror("Waitpid error");
                exit(EXIT_FAILURE);
            }
        }

        // Add the command to history immediately before exiting the function
        if (history_count < MAX_HIS)
        {
            strncpy(history[history_count].cmd, cmd, MAX_SIZE);
            history[history_count].pid = child_pid;
            time(&history[history_count].start_time);
            history[history_count].background = background;
            history_count++; // Increment the history_count
        }
        else
        {
            fprintf(stderr, "History is full. Cannot add more entries.\n");
        }
    }
    return 0;
}

// Function to trim leading and trailing whitespace from a string
void trim_whitespace(char *str)
{
    int len = strlen(str);
    int start = 0, end = len - 1;

    // Find the first non-whitespace character from the beginning
    while (isspace(str[start]))
    {
        start++;
    }

    // Find the last non-whitespace character from the end
    while (end >= 0 && isspace(str[end]))
    {
        end--;
    }

    // Shift characters to the left to remove leading whitespace
    int i, j;
    for (i = start, j = 0; i <= end; i++, j++)
    {
        str[j] = str[i];
    }

    // Null-terminate the trimmed string
    str[j] = '\0';
}

// Function to update history if piped command
int history_piped_commands(char *cmd)
{
    char *token;
    char *saveptr;

    // Tokenize the input string using "|" as the delimiter
    token = strtok_r(cmd, "|", &saveptr);

    int foreground = 0; // Flag to track whether a command is foreground or background

    while (token != NULL)
    {
        // Trim leading and trailing whitespace from the token
        trim_whitespace(token);

        int pipe_fd[2];
        pid_t pid;

        if (pipe(pipe_fd) == -1)
        {
            perror("Pipe error");
            exit(EXIT_FAILURE);
        }

        pid = fork();

        if (pid < 0)
        {
            perror("Fork error");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        { // Child process (writes to the pipe)
            close(pipe_fd[0]); // Close the read end of the pipe
            dup2(pipe_fd[1], STDOUT_FILENO); // Redirect stdout to the pipe
            close(pipe_fd[1]);

            // You can execute the command here if needed
            // But for this modified version, we won't execute it

            exit(EXIT_SUCCESS);
        }
        else
        { // Parent process
            close(pipe_fd[1]); // Close the write end of the pipe

            // Add the command to history
            if (history_count < MAX_HIS)
            {
                strncpy(history[history_count].cmd, token, MAX_SIZE);
                history[history_count].pid = pid;
                time(&history[history_count].start_time);
                history[history_count].background = foreground;
                history_count++;
            }
            else
            {
                printf("History is full. Cannot add more entries.\n");
            }

            waitpid(pid, NULL, 0);
        }

        token = strtok_r(NULL, "|", &saveptr);
        foreground = 1; // Set to 1 after the first command
    }

    return 1;
}

// Function to execute piped commands
int execute_piped_commands(char *cmd)
{
    int pipe_fd[2];
    pid_t pid1, pid2;

    if (pipe(pipe_fd) == -1)
    {
        perror("Pipe error");
        exit(EXIT_FAILURE);
    }

    pid1 = fork();

    if (pid1 < 0)
    {
        perror("Fork error");
        exit(EXIT_FAILURE);
    }
    else if (pid1 == 0)
    { // Child process 1 (writes to the pipe)
        close(pipe_fd[0]); // Close the read end of the pipe
        if (dup2(pipe_fd[1], STDOUT_FILENO) == -1)
        {
            perror("dup2 error");
            exit(EXIT_FAILURE);
        }
        close(pipe_fd[1]);

        // Execute the first command
        if (system(cmd) == -1)
        {
            perror("System error");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    else
    { // Parent process
        pid2 = fork();

        if (pid2 < 0)
        {
            perror("Fork error");
            exit(EXIT_FAILURE);
        }
        else if (pid2 == 0)
        { // Child process 2 (reads from the pipe)
            close(pipe_fd[1]); // Close the write end of the pipe
            if (dup2(pipe_fd[0], STDIN_FILENO) == -1)
            {
                perror("dup2 error");
                exit(EXIT_FAILURE);
            }
            close(pipe_fd[0]);

            // Execute the second command
            if (system(cmd) == -1)
            {
                perror("System error");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }
        else
        { // Parent process
            close(pipe_fd[0]);
            close(pipe_fd[1]);

            if (waitpid(pid1, NULL, 0) == -1)
            {
                perror("waitpid error");
                exit(EXIT_FAILURE);
            }
            if (waitpid(pid2, NULL, 0) == -1)
            {
                perror("waitpid error");
                exit(EXIT_FAILURE);
            }
        }
    }
    history_piped_commands(cmd);
    return 1;
}

// Signal handler for Ctrl+C
void sigint_handler(int signo)
{
    printf("\nExiting the shell...........\n");
    display_history();
    exit(0);
}

// Setup signal handler for Ctrl+C
void setup_signal_handler()
{
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    if (sigemptyset(&sa.sa_mask) == -1)
    {
        perror("sigemptyset error");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction error");
        exit(EXIT_FAILURE);
    }
}

// Main shell loop
void shell_loop()
{
    int status;
    char input[MAX_SIZE];

    do
    {
        printf("aryan_parth@simpleShell:~$ ");
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            perror("fgets error");
            exit(EXIT_FAILURE);
        }
        input[strcspn(input, "\n")] = '\0';

        // Check for piping operator
        char *pipe_ptr = strchr(input, '|');
        if (pipe_ptr != NULL)
        {
            status = execute_piped_commands(input);
        }
        else
        {
            status = launch(input);
        }
    } while (status);
}

// Function to launch a command
int launch(char *cmd)
{
    if (strcmp(cmd, "history") == 0)
    {
        display_history();
    }
    else if (strcmp(cmd, "exit") == 0)
    {
        display_history();
        printf("\nShell ended successfully!\n");
        return 0;
    }
    else if (strstr(cmd, "&") != NULL)
    {
        char cmd_copy[MAX_SIZE];
        strncpy(cmd_copy, cmd, sizeof(cmd_copy)); // Create a copy of the original command

        // Tokenize the input string using "&" as the delimiter
        char *token = strtok(cmd_copy, "&"); // Split the input by "&"
        int foreground = 0; // Flag to track whether a command is foreground or background

        while (token != NULL)
        {
            // Trim leading and trailing whitespace from the token
            trim_whitespace(token);
            if (create_process_and_run(token, foreground) == -1)
            {
                fprintf(stderr, "Error launching command: %s\n", token);
                return 1;
            }
            foreground = 1; // Set to 1 after the first command

            // Get the next token
            token = strtok(NULL, "&");
        }
    }
    else
    {
        if (create_process_and_run(cmd, 0) == -1)
        {
            fprintf(stderr, "Error launching command: %s\n", cmd);
            return 1;
        }
    }
    return 1;
}

// Function to display command history
void display_history()
{
    printf("\nCommand History:\n");
    for (int i = 0; i < history_count; i++)
    {
        printf("[%d] PID: %d - %s\n", i + 1, history[i].pid, history[i].cmd);
        printf("Start Time: %s", ctime(&history[i].start_time));
        if (history[i].background)
        {
            printf("Background Process\n");
        }
        else
        {
            printf("Execution Duration: %ld seconds\n", time(NULL) - history[i].start_time);
        }
        printf("\n"); // Add a newline for better formatting
    }
}