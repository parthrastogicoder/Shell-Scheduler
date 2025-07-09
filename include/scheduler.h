#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

#define MAX_PROCESSES 100
#define MAX_PROGRAM_NAME 256

// Structure to represent a process
struct Process {
    pid_t pid;
    char command[MAX_PROGRAM_NAME];
    int state; // 0: Running, 1: Waiting, -1: Finished
    struct timeval start_time;
    struct timeval end_time;
    long long total_execution_time;
    long long waiting_time;
    int priority;
};

// Structure to represent a queue of processes
struct ProcessQueue {
    struct Process processes[MAX_PROCESSES];
    int rear;
};

// Structure for terminated processes
struct TerminatedQueue {
    struct Process processes[MAX_PROCESSES];
    int rear;
};

// Function declarations
void enqueue(struct ProcessQueue* queue, struct Process process);
void handleSIGUSR1(int signo);
void printTerminatedQueue(struct TerminatedQueue* queue);
void handleSIGCHLD(int signo);
int run_basic_scheduler(void);
int run_advanced_scheduler(void);

#endif // SCHEDULER_H
